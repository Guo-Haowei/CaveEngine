#include "cave/runtime/core/time/Nanoseconds.h"

namespace cave::time {

TEST(Nanoseconds, operator_add) {
    Nanoseconds ns = Nanoseconds(10) + Nanoseconds(20);
    EXPECT_EQ(ns.value, 30);
}

TEST(Nanoseconds, operator_sub) {
    Nanoseconds ns = Nanoseconds(100) - Nanoseconds(20);
    EXPECT_EQ(ns.value, 80);
}

TEST(Nanoseconds, operator_add_assign) {
    Nanoseconds ns(100);
    ns += Nanoseconds(90);
    EXPECT_EQ(ns.value, 190);
}

TEST(Nanoseconds, operator_sub_assign) {
    Nanoseconds ns(100);
    ns -= Nanoseconds(90);
    EXPECT_EQ(ns.value, 10);
}

}  // namespace cave::time
