#include "cave/core/algorithm/Graph.h"

namespace cave {

TEST(TopologicalSort, SingleEdge) {
    Vector<TopoSortEdge> edges = { { 1, 0 } };

    auto sorted_opt = TopologicalSort(2, edges);
    ASSERT_TRUE(sorted_opt.is_some());
    auto sorted = sorted_opt.unwrap_unchecked();

    EXPECT_EQ(sorted[0], 1);
    EXPECT_EQ(sorted[1], 0);
}

TEST(TopologicalSort, Diamond) {
    Vector<TopoSortEdge> edges = {
        { 0, 1 },
        { 0, 2 },
        { 1, 3 },
        { 2, 3 },
    };

    auto sorted_opt = TopologicalSort(4, edges);
    ASSERT_TRUE(sorted_opt.is_some());
    auto sorted = sorted_opt.unwrap_unchecked();

    EXPECT_EQ(sorted[0], 0);
    EXPECT_EQ(sorted[1], 1);
    EXPECT_EQ(sorted[2], 2);
    EXPECT_EQ(sorted[3], 3);
}

TEST(TopologicalSort, Cycle) {
    Vector<TopoSortEdge> edges = {
        { 0, 1 },
        { 1, 2 },
        { 2, 3 },
        { 3, 1 },
    };

    auto sorted_opt = TopologicalSort(4, edges);
    ASSERT_TRUE(sorted_opt.is_none());
}

TEST(TopologicalSort, LargeGraph) {
    Vector<TopoSortEdge> edges = {
        { 0, 2 },
        { 0, 3 },
        { 0, 4 },

        { 1, 3 },
        { 1, 4 },

        { 2, 5 },
        { 3, 5 },
        { 3, 6 },
        { 4, 6 },

        { 5, 7 },
        { 6, 8 },
        { 7, 8 },
    };

    auto sorted_opt = TopologicalSort(9, edges);

    ASSERT_TRUE(sorted_opt.is_some());
    auto sorted = sorted_opt.unwrap_unchecked();

    EXPECT_EQ(sorted[0], 0);
    EXPECT_EQ(sorted[1], 1);
    EXPECT_EQ(sorted[2], 2);
    EXPECT_EQ(sorted[3], 3);
    EXPECT_EQ(sorted[4], 4);
    EXPECT_EQ(sorted[5], 5);
    EXPECT_EQ(sorted[6], 6);
    EXPECT_EQ(sorted[7], 7);
    EXPECT_EQ(sorted[8], 8);
}

TEST(FindConnectedTiles, FillTiles) {
    constexpr int width = 8;
    constexpr int height = 6;
    std::array<uint16_t, width * height> tiles = {
        1, 1, 1, 1, 1, 1, 1, 1,  // 0-7
        1, 0, 1, 0, 1, 1, 1, 1,  // 8-15
        1, 0, 1, 0, 1, 1, 1, 1,  // 16-23
        1, 0, 1, 0, 0, 1, 1, 1,  // 24-31
        1, 0, 0, 0, 1, 1, 1, 1,  // 32-39
        1, 1, 1, 1, 1, 1, 1, 1,  // 40-47
    };

    auto result = FindConnectedTiles(tiles, width, height, 9);
    HashSet<uint16_t> set(result.begin(), result.end());
    EXPECT_EQ(set.size(), 10);
    EXPECT_TRUE(set.contains(9));
    EXPECT_TRUE(set.contains(11));

    EXPECT_TRUE(set.contains(17));
    EXPECT_TRUE(set.contains(19));

    EXPECT_TRUE(set.contains(25));
    EXPECT_TRUE(set.contains(27));
    EXPECT_TRUE(set.contains(28));

    EXPECT_TRUE(set.contains(33));
    EXPECT_TRUE(set.contains(34));
    EXPECT_TRUE(set.contains(35));
}

}  // namespace cave
