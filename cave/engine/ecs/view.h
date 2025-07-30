#pragma once
#include "component_manager.h"

namespace cave::ecs {

template<bool IsConst, class T>
using MaybeConst = std::conditional_t<IsConst, const T, T>;

template<bool IsConst, class T>
using MaybeRef = std::conditional_t<IsConst, const T&, T&>;

template<bool IsConst, class... Cs>
class BasicView {
    using MgrTuple = std::tuple<MaybeConst<IsConst, ComponentManager<Cs>>*...>;

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
                 MgrTuple mgrs)
            : m_i(i), m_ents(ents), m_mgrs(mgrs) {
            m_n = m_ents ? m_ents->size() : 0;
            skip_to_valid();
        }

        value_type operator*() const {
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
            return std::tuple<MaybeRef<IsConst, Cs>...>(
                std::get<I>(m_mgrs)->GetComponentByIndex(index_of<I>(e))...);
        }
        auto refs_for(Entity e) const {
            return refs_for_impl(e, std::index_sequence_for<Cs...>{});
        }

        bool present_in_all(Entity e) const {
            return present_in_all_impl(e, std::index_sequence_for<Cs...>{});
        }

        template<std::size_t... I>
        bool present_in_all_impl(Entity e, std::index_sequence<I...>) const {
            bool ok = true;
            ((ok = ok && std::get<I>(m_mgrs)->Contains(e)), ...);
            return ok;
        }

        template<std::size_t I>
        std::size_t index_of(Entity e) const {
            auto idx = std::get<I>(m_mgrs)->FindIndex(e);
            DEV_ASSERT(idx.is_some());
            return idx.unwrap();
        }

        void skip_to_valid() {
            while (m_i < m_n) {
                const Entity e = (*m_ents)[m_i];
                if (present_in_all(e)) break;
                ++m_i;
            }
        }

        size_t m_i = 0;
        size_t m_n = 0;
        const std::vector<Entity>* m_ents = nullptr;
        MgrTuple m_mgrs{};
    };

    explicit BasicView(MaybeConst<IsConst, ComponentManager<Cs>>&... mgrs)
        : m_mgrs{ (&mgrs)... } {
        pick_baseline();
    }

    iterator begin() { return iterator(0, m_baseline, m_mgrs); }
    iterator end() { return iterator(m_baseline_size, m_baseline, m_mgrs); }
    iterator begin() const { return iterator(0, m_baseline, m_mgrs); }
    iterator end() const { return iterator(m_baseline_size, m_baseline, m_mgrs); }

private:
    void pick_baseline() {
        pick_baseline_impl(std::index_sequence_for<Cs...>{});
    }

    template<std::size_t... I>
    void pick_baseline_impl(std::index_sequence<I...>) {
        std::array<std::size_t, sizeof...(Cs)> counts{ (std::get<I>(m_mgrs)->GetCount())... };
        std::array<const std::vector<Entity>*, sizeof...(Cs)> ents{ (&std::get<I>(m_mgrs)->GetEntityArray())... };

        std::size_t minIdx = 0;
        for (std::size_t i = 1; i < counts.size(); ++i) {
            if (counts[i] < counts[minIdx]) minIdx = i;
        }
        m_baseline = ents[minIdx];
        m_baseline_size = counts[minIdx];
    }

private:
    MgrTuple m_mgrs{};
    const std::vector<Entity>* m_baseline = nullptr;
    std::size_t m_baseline_size = 0;
};

// Convenient aliases
template<class... Cs>
using View = BasicView<false, Cs...>;

template<class... Cs>
using ConstView = BasicView<true, Cs...>;

}  // namespace cave::ecs
