#include "cave/core/string/StringId.h"

namespace cave::string_utils {

TEST(StringId, default_constructed_id_has_zero_hash) {
    StringId id;

    EXPECT_EQ(id.hash(), 0u);
}

#if !USING(STRING_ID_KEEP_SOURCE)
TEST(StringId, can_be_constructed_at_compile_time) {
    constexpr StringId id1("abcd");
    constexpr StringId id2("abcd");
    constexpr StringId id3("efgh");

    static_assert(id1.hash() != 0);
    static_assert(id1 == id2);
    static_assert(id1 != id3);
}
#endif

TEST(StringId, same_string_is_equal) {
    struct Case {
        std::string_view value;
    };

    const Case cases[] = {
        { "ui_left" },
        { "super_long_id_1@#$%" },
    };

    for (const Case& c : cases) {
        SCOPED_TRACE(c.value);

        StringId id1(c.value);
        StringId id2(c.value);

        EXPECT_EQ(id1, id2);
    }
}

TEST(StringId, different_strings_are_not_equal) {
    struct Case {
        std::string_view a;
        std::string_view b;
    };

    const Case cases[] = {
        { "ui_left", "ui_right" },
        { "super_long_id", "super_long_id_1@#$%" },
    };

    for (const Case& c : cases) {
        SCOPED_TRACE(c.a);

        StringId id1(c.a);
        StringId id2(c.b);

        EXPECT_NE(id1, id2);
    }
}

TEST(StringId, same_string_has_same_hash) {
    struct Case {
        std::string_view value;
    };

    const Case cases[] = {
        { "jump_released" },
        { "1231&*)@#$%" },
    };

    for (const Case& c : cases) {
        SCOPED_TRACE(c.value);

        StringId id1(c.value);
        StringId id2(c.value);

        EXPECT_EQ(id1.hash(), id2.hash());
    }
}

TEST(StringId, different_strings_usually_have_different_hashes) {
    struct Case {
        std::string_view a;
        std::string_view b;
    };

    const Case cases[] = {
        { "jump_released", "jump_pressed" },
        { "random string", "1231&*)@#$%" },
    };

    for (const Case& c : cases) {
        SCOPED_TRACE(c.a);

        StringId id1(c.a);
        StringId id2(c.b);

        EXPECT_NE(id1.hash(), id2.hash());
    }
}

TEST(StringId, std_hash_returns_string_id_hash) {
    StringId id("ui_left");

    EXPECT_EQ(std::hash<StringId>{}(id), id.hash());
}

#if USING(STRING_ID_KEEP_SOURCE)
TEST(StringId, debug_name_preserves_source_string) {
    StringId id("ui_left");

    EXPECT_EQ(id.debugName(), "ui_left");
}

TEST(StringId, debug_name_trims_long_source_string) {
    StringId id1("this_is_a_very_long_string_id_name_that_exceeds_32_chars");
    StringId id2("this_is_a_very_long_string_id_n");

    EXPECT_FALSE(id1.debugName().empty());
    EXPECT_NE(id1.hash(), id2.hash());
    EXPECT_EQ(id1.debugName(), id2.debugName());
}
#endif

}  // namespace cave::string_utils
