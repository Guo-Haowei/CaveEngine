#include "cave/core/ids/GenId.h"

namespace cave {

using TestId = GenId<int>;

TEST(GenId, valid_when_gen_is_not_zero) {
    TestId id1{ 2, 8 };
    TestId id2{ 2, 0 };
    EXPECT_TRUE(id1.valid());
    EXPECT_FALSE(id2.valid());
}

TEST(GenId, only_equal_when_id_and_gen_both_equal) {
    TestId id1{ 1, 2 };
    TestId id2{ 1, 2 };
    EXPECT_EQ(id1, id2);

    TestId id3{ 1, 3 };
    TestId id4{ 2, 2 };
    EXPECT_NE(id1, id3);
    EXPECT_NE(id1, id4);
}

TEST(GenId, can_be_used_with_hash_map) {
    std::unordered_map<TestId, int> map;
    TestId id1(1, 2);
    TestId id2(8, 9);
    TestId id3(9, 9);
    map.emplace(id1, 4);
    map.emplace(id2, 6);
    {
        auto it = map.find(id1);
        EXPECT_NE(it, map.end());
        EXPECT_EQ(it->second, 4);
    }
    {
        auto it = map.find(id3);
        EXPECT_EQ(it, map.end());
    }
    map.emplace(id3, 8);
    {
        auto it = map.find(id3);
        EXPECT_NE(it, map.end());
        EXPECT_EQ(it->second, 8);
    }
    map[id1] = 100;
    {
        auto it = map.find(id1);
        EXPECT_NE(it, map.end());
        EXPECT_EQ(it->second, 100);
    }
}

}  // namespace cave
