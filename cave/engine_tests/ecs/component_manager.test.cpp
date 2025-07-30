#include "engine/ecs/component_manager.inl"

namespace cave::ecs {

// @TODO: move to view
namespace detail {

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
        // Build (Cs&... / const Cs&...) tuple for entity e
        template<std::size_t... I>
        auto refs_for_impl(Entity e, std::index_sequence<I...>) const {
            return std::tuple<MaybeRef<IsConst, Cs>...>(
                std::get<I>(m_mgrs)->GetComponentByIndex(index_of<I>(e))...);
        }
        auto refs_for(Entity e) const {
            return refs_for_impl(e, std::index_sequence_for<Cs...>{});
        }

        // Present in all managers?
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
        // Collect sizes and entity arrays from each manager
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

struct C1 {
    int a = 0;
    void OnDeserialized() {}
};
struct C2 {
    float b = 0.0f;
    void OnDeserialized() {}
};
struct C3 {
    int c = 0;
    void OnDeserialized() {}
};

template<ComponentType T>
class MockManager : public ComponentManager<T> {
public:
    void Add(Entity p_entity, const T& p_component) {
        const size_t index = ComponentManager<T>::m_componentArray.size();
        ComponentManager<T>::m_lookup[p_entity] = index;
        ComponentManager<T>::m_entityArray.emplace_back(p_entity);
        ComponentManager<T>::m_componentArray.emplace_back(p_component);
    }
};

TEST(view, single_manager) {
    MockManager<C1> m1;

    // m1: {1,2,3,5}
    m1.Add(Entity(1), C1{ 10 });
    m1.Add(Entity(2), C1{ 20 });
    m1.Add(Entity(3), C1{ 30 });
    m1.Add(Entity(5), C1{ 50 });

    View<C1> v(m1);
    std::vector<std::tuple<Entity, int, float>> got;

    for (auto [e, c1] : v) {
        // mutate to verify non-const
        c1.a += 1;
    }

    EXPECT_EQ(m1.GetComponent(Entity(1))->a, 11);
    EXPECT_EQ(m1.GetComponent(Entity(2))->a, 21);
    EXPECT_EQ(m1.GetComponent(Entity(3))->a, 31);
    EXPECT_EQ(m1.GetComponent(Entity(5))->a, 51);
}

TEST(view, intersection_two_managers) {
    MockManager<C1> m1;
    MockManager<C2> m2;

    // m1: {1,2,3,5}
    m1.Add(Entity(1), C1{ 10 });
    m1.Add(Entity(2), C1{ 20 });
    m1.Add(Entity(3), C1{ 30 });
    m1.Add(Entity(5), C1{ 50 });

    // m2: {2,3,4}
    m2.Add(Entity(2), C2{ 2.0f });
    m2.Add(Entity(3), C2{ 3.0f });
    m2.Add(Entity(4), C2{ 4.0f });

    // Intersection: {2,3}, baseline is m2 (smaller)
    View<C1, C2> v(m1, m2);

    std::vector<std::tuple<Entity, int, float>> got;
    for (auto [e, c1, c2] : v) {
        got.emplace_back(e, c1.a, c2.b);
        // mutate to verify non-const
        c1.a += 1;
        c2.b += 0.5f;
    }

    ASSERT_EQ(got.size(), 2u);
    EXPECT_EQ(std::get<0>(got[0]), Entity(2));
    EXPECT_EQ(std::get<1>(got[0]), 20);
    EXPECT_FLOAT_EQ(std::get<2>(got[0]), 2.0f);

    EXPECT_EQ(std::get<0>(got[1]), Entity(3));
    EXPECT_EQ(std::get<1>(got[1]), 30);
    EXPECT_FLOAT_EQ(std::get<2>(got[1]), 3.0f);

    EXPECT_EQ(m1.GetComponentByIndex(m1.FindIndex(Entity(2)).unwrap()).a, 21);
    EXPECT_FLOAT_EQ(m2.GetComponentByIndex(m2.FindIndex(Entity(2)).unwrap()).b, 2.5f);
    EXPECT_EQ(m1.GetComponentByIndex(m1.FindIndex(Entity(3)).unwrap()).a, 31);
    EXPECT_FLOAT_EQ(m2.GetComponentByIndex(m2.FindIndex(Entity(3)).unwrap()).b, 3.5f);
}

TEST(view, intersection_three_managers) {
    MockManager<C1> m1;
    MockManager<C2> m2;
    MockManager<C3> m3;

    // m1: {10,11,12}
    m1.Add(Entity(10), C1{ 1 });
    m1.Add(Entity(11), C1{ 2 });
    m1.Add(Entity(12), C1{ 3 });
    // m2: {11,12,13}
    m2.Add(Entity(11), C2{ 1.1f });
    m2.Add(Entity(12), C2{ 1.2f });
    m2.Add(Entity(13), C2{ 1.3f });
    // m3: {12,14}
    m3.Add(Entity(12), C3{ 42 });
    m3.Add(Entity(14), C3{ 99 });

    View<C1, C2, C3> v(m1, m2, m3);
    std::vector<std::tuple<Entity, int, float, int>> got;
    for (auto [e, c1, c2, c3] : v) {
        got.emplace_back(e, c1.a, c2.b, c3.c);
    }
    ASSERT_EQ(got.size(), 1u);
    EXPECT_EQ(std::get<0>(got[0]), Entity(12));
    EXPECT_EQ(std::get<1>(got[0]), 3);
    EXPECT_FLOAT_EQ(std::get<2>(got[0]), 1.2f);
    EXPECT_EQ(std::get<3>(got[0]), 42);
}

TEST(view, const_view_is_read_only) {
    MockManager<C1> m1;
    MockManager<C2> m2;
    m1.Add(Entity(1), C1{ 5 });
    m2.Add(Entity(1), C2{ 7.0f });

    ConstView<C1, C2> cv(m1, m2);
    for (auto [e, c1, c2] : cv) {
        (void)e;
        static_assert(std::is_const_v<std::remove_reference_t<decltype(c1)>>, "c1 must be const");
        static_assert(std::is_const_v<std::remove_reference_t<decltype(c2)>>, "c2 must be const");
#if 0
        c1.a = 10;  // would fail to compile
#endif
    }
}

TEST(view, empty_intersection) {
    MockManager<C1> m1;
    MockManager<C2> m2;
    m1.Add(Entity(1), C1{});
    m1.Add(Entity(2), C1{});
    m2.Add(Entity(3), C2{});
    m2.Add(Entity(4), C2{});

    View<C1, C2> v(m1, m2);
    std::size_t count = 0;
    for (auto&& item : v) {
        (void)item;
        ++count;
    }
    EXPECT_EQ(count, 0u);
}

TEST(view, baseline_is_smallest_set) {
    MockManager<C1> m1;
    MockManager<C2> m2;

    for (uint32_t e = 1; e <= 5; ++e) {
        m1.Add(Entity(e), C1{ int(e) });
    }

    m2.Add(Entity(2), C2{ 2.0f });
    m2.Add(Entity(4), C2{ 4.0f });

    View<C1, C2> v(m1, m2);
    std::vector<Entity> got;
    for (auto [e, c1, c2] : v) {
        got.push_back(e);
        (void)c1;
        (void)c2;
    }

    ASSERT_EQ(got.size(), 2u);
    EXPECT_EQ(got[0], Entity(2));
    EXPECT_EQ(got[1], Entity(4));
}

}  // namespace detail

}  // namespace cave::ecs
