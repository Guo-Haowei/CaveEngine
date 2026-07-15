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

}  // namespace cave
