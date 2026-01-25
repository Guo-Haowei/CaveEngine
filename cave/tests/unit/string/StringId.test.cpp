#include "cave/runtime/string/StringId.h"

namespace cave::string_utils {

#if USING(STRING_ID_KEEKP_SOURCE)
#define CONSTEXPR
#else
#define CONSTEXPR constexpr
#endif

TEST(StringId, comparison) {
    CONSTEXPR StringId id1("ui_left");
    CONSTEXPR StringId id2("ui_left");
    CONSTEXPR StringId id3("ui_right");

    EXPECT_EQ(id1, id2);
    EXPECT_NE(id1, id3);
}

TEST(StringId, use_with_unordered_map) {
    std::unordered_map<StringId, int> map;
    map[StringId("ui_up")] = 1;
    map[StringId("ui_down")] = 2;
    EXPECT_EQ(map.size(), 2);
    map[StringId("ui_down")] = 3;
    EXPECT_EQ(map.size(), 2);
    map[StringId("ui_right")] = 4;
    EXPECT_EQ(map.size(), 3);
    EXPECT_EQ(map[StringId("ui_down")], 3);
}

TEST(StringId, use_with_map) {
    std::map<StringId, int> map;
    map[StringId("ui_up")] = 1;
    map[StringId("ui_down")] = 2;
    EXPECT_EQ(map.size(), 2);
    map[StringId("ui_down")] = 3;
    EXPECT_EQ(map.size(), 2);
    map[StringId("ui_right")] = 4;
    EXPECT_EQ(map.size(), 3);
    EXPECT_EQ(map[StringId("ui_down")], 3);
}

}  // namespace cave::string_utils
