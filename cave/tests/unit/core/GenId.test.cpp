#include "cave/runtime/core/GenId.h"

namespace cave {

using TestId = GenId<int>;

TEST(GenId, valid_when_gen_is_not_zero) {
    TestId id1{ 2, 8 };
    TestId id2{ 2, 0 };
    EXPECT_TRUE(id1.IsValid());
    EXPECT_FALSE(id2.IsValid());
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

}  // namespace cave
