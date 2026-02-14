#pragma once
#include "ComponentPool.h"

namespace cave::ecs {

template<bool IsConst, class T>
using MaybeConst = std::conditional_t<IsConst, const T, T>;

template<bool IsConst, class T>
using MaybeRef = std::conditional_t<IsConst, const T&, T&>;

template<bool IsConst, class... Cs>
class BasicView {
    using PoolPtrTuple = std::tuple<MaybeConst<IsConst, ComponentPool<Cs>>*...>;

public:
    using value_type = std::tuple<Entity, MaybeRef<IsConst, Cs>...>;

    class iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename BasicView::value_type;

        iterator() = default;

        iterator(std::size_t i,
                 const std::vector<Entity>* ents,
                 PoolPtrTuple pools)
            : m_i(i)
            , m_ents(ents)
            , m_pools(pools) {
            m_n = m_ents ? m_ents->size() : 0;
            skip_to_valid();
        }

        value_type operator*() const {
            DEV_ASSERT(m_ents != nullptr);
            DEV_ASSERT(m_i < m_n);
            const Entity e = (*m_ents)[m_i];
            return std::tuple_cat(std::make_tuple(e), refs_for(e));
        }

        iterator& operator++() {
            ++m_i;
            skip_to_valid();
            return *this;
        }

        iterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& r) const {
            return m_i == r.m_i && m_ents == r.m_ents;
        }

        bool operator!=(const iterator& r) const { return !(*this == r); }

    private:
        template<std::size_t... I>
        auto refs_for_impl(Entity e, std::index_sequence<I...>) const {
            // Safe because view guarantees "no null pools" when m_ents != nullptr.
            DEV_ASSERT(all_pools_non_null());
            return std::tuple<MaybeRef<IsConst, Cs>...>(
                std::get<I>(m_pools)->GetComponentByIndex(index_of<I>(e))...);
        }

        auto refs_for(Entity e) const {
            return refs_for_impl(e, std::index_sequence_for<Cs...>{});
        }

        bool present_in_all(Entity e) const {
            return present_in_all_impl(e, std::index_sequence_for<Cs...>{});
        }

        template<std::size_t... I>
        bool present_in_all_impl(Entity e, std::index_sequence<I...>) const {
            // Safe because view guarantees "no null pools" when m_ents != nullptr.
            bool ok = true;
            ((ok = ok && std::get<I>(m_pools)->Has(e)), ...);
            return ok;
        }

        template<std::size_t I>
        std::size_t index_of(Entity e) const {
            auto idx = std::get<I>(m_pools)->FindIndex(e);
            DEV_ASSERT(idx.is_some());
            return idx.unwrap();
        }

        void skip_to_valid() {
            // Empty view => m_ents == nullptr or size == 0, loop never runs.
            while (m_i < m_n) {
                const Entity e = (*m_ents)[m_i];
                if (present_in_all(e)) break;
                ++m_i;
            }
        }

        bool all_pools_non_null() const {
            return all_pools_non_null_impl(std::index_sequence_for<Cs...>{});
        }

        template<std::size_t... I>
        bool all_pools_non_null_impl(std::index_sequence<I...>) const {
            bool ok = true;
            ((ok = ok && (std::get<I>(m_pools) != nullptr)), ...);
            return ok;
        }

    private:
        std::size_t m_i = 0;
        std::size_t m_n = 0;
        const std::vector<Entity>* m_ents = nullptr;
        PoolPtrTuple m_pools{};
    };

public:
    // Any null pool => empty view.
    explicit BasicView(MaybeConst<IsConst, ComponentPool<Cs>>*... pools)
        : m_pools{ pools... } {
        pick_baseline_or_empty();
    }

    iterator begin() { return iterator(0, m_baseline, m_pools); }
    iterator end() { return iterator(m_baseline_size, m_baseline, m_pools); }
    iterator begin() const { return iterator(0, m_baseline, m_pools); }
    iterator end() const { return iterator(m_baseline_size, m_baseline, m_pools); }

private:
    void pick_baseline_or_empty() {
        // If any pool pointer is null, the view is empty.
        if (!all_pools_non_null()) {
            m_baseline = nullptr;
            m_baseline_size = 0;
            return;
        }

        // All pools non-null: pick the smallest entity array as baseline.
        pick_baseline_impl(std::index_sequence_for<Cs...>{});
    }

    bool all_pools_non_null() const {
        return all_pools_non_null_impl(std::index_sequence_for<Cs...>{});
    }

    template<std::size_t... I>
    bool all_pools_non_null_impl(std::index_sequence<I...>) const {
        bool ok = true;
        ((ok = ok && (std::get<I>(m_pools) != nullptr)), ...);
        return ok;
    }

    template<std::size_t... I>
    void pick_baseline_impl(std::index_sequence<I...>) {
        // Baseline: choose pool with smallest entity array.
        const std::vector<Entity>* ents[] = { &std::get<I>(m_pools)->GetEntityArray()... };
        std::size_t sizes[] = { std::get<I>(m_pools)->GetEntityArray().size()... };

        std::size_t minIdx = 0;
        for (std::size_t i = 1; i < sizeof...(Cs); ++i) {
            if (sizes[i] < sizes[minIdx]) minIdx = i;
        }

        m_baseline = ents[minIdx];
        m_baseline_size = sizes[minIdx];
    }

private:
    PoolPtrTuple m_pools{};
    const std::vector<Entity>* m_baseline = nullptr;
    std::size_t m_baseline_size = 0;
};

template<class... Cs>
using View = BasicView<false, Cs...>;

template<class... Cs>
using ConstView = BasicView<true, Cs...>;

}  // namespace cave::ecs
