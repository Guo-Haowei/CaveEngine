#include "engine/ecs/component_manager.inl"
#include "engine/ecs/view.h"

namespace cave {

struct C1 {
    int a = 0;
};

struct C2 {
    float b = 0.0f;
};

struct C3 {
    int c = 0;
};

template<>
struct IsComponent<C1> : std::true_type {};

template<>
struct IsComponent<C2> : std::true_type {};

template<>
struct IsComponent<C3> : std::true_type {};

}  // namespace cave

namespace cave::ecs {

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

}  // namespace cave::ecs
