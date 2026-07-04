#include "cave/core/string/StringUtils.h"

namespace cave::string {

TEST(StringUtils_IsNullOrEmpty, returns_true_for_nullptr) {
    EXPECT_TRUE(StringUtils::isNullOrEmpty(nullptr));
}

TEST(StringUtils_IsNullOrEmpty, returns_true_for_empty_string) {
    EXPECT_TRUE(StringUtils::isNullOrEmpty(""));
}

TEST(StringUtils_IsNullOrEmpty, returns_false_for_non_empty_string) {
    EXPECT_FALSE(StringUtils::isNullOrEmpty("abc"));
}

TEST(StringUtils_Equal, treats_nullptr_and_nullptr_as_equal) {
    EXPECT_TRUE(StringUtils::equal(nullptr, nullptr));
}

TEST(StringUtils_Equal, treats_empty_and_nullptr_as_equal) {
    EXPECT_TRUE(StringUtils::equal("", nullptr));
}

TEST(StringUtils_Equal, returns_false_when_nullptr_compared_with_non_empty) {
    EXPECT_FALSE(StringUtils::equal(nullptr, "abc"));
}

TEST(StringUtils_Equal, treats_nullptr_and_empty_as_equal) {
    EXPECT_TRUE(StringUtils::equal(nullptr, ""));
}

TEST(StringUtils_Equal, treats_empty_and_empty_as_equal) {
    EXPECT_TRUE(StringUtils::equal("", ""));
}

TEST(StringUtils_Equal, returns_false_when_non_empty_compared_with_empty) {
    EXPECT_FALSE(StringUtils::equal("abc", ""));
}

TEST(StringUtils_Equal, returns_true_for_same_strings) {
    EXPECT_TRUE(StringUtils::equal("abcd", "abcd"));
    EXPECT_TRUE(StringUtils::equal("01234", "01234"));
}

TEST(StringUtils_Equal, returns_false_for_different_strings) {
    EXPECT_FALSE(StringUtils::equal("acd", "abcd"));
    EXPECT_FALSE(StringUtils::equal("01", "01234"));
}

TEST(StringUtils_ReplaceFirst, does_nothing_when_pattern_not_found) {
    std::string str = "123_123";
    StringUtils::replaceFirst(str, "1234", "321");
    EXPECT_EQ(str, "123_123");
}

TEST(StringUtils_ReplaceFirst, replaces_only_first_matching_pattern) {
    std::string str = "123_123";
    StringUtils::replaceFirst(str, "123", "321");
    EXPECT_EQ(str, "321_123");
}

TEST(StringUtils_Sprintf, formats_string) {
    char buffer[64];
    StringUtils::sprintf(buffer, "(%d %c %s = %u)", 1, '+', "10", 11);

    EXPECT_STREQ(buffer, "(1 + 10 = 11)");
}

TEST(StringUtils_Sprintf, truncates_when_buffer_is_too_small) {
    char buffer[8];
    StringUtils::sprintf(buffer, "12%s", "345678");

    EXPECT_STREQ(buffer, "1234567");
}

TEST(StringUtils_Sprintf, writes_full_string_when_buffer_is_large_enough) {
    char buffer[9];
    StringUtils::sprintf(buffer, "12%s", "345678");

    EXPECT_STREQ(buffer, "12345678");
}

TEST(StringUtils_Strcpy, copies_full_string_when_buffer_is_large_enough) {
    char buffer[8];
    StringUtils::strcpy(buffer, "abcdefg");

    EXPECT_STREQ(buffer, "abcdefg");
}

TEST(StringUtils_Strcpy, truncates_when_buffer_is_too_small) {
    char buffer[5];
    StringUtils::strcpy(buffer, "abcdefg");

    EXPECT_STREQ(buffer, "abcd");
}

TEST(StringSplitter, returns_no_tokens_for_null_string) {
    const char* source = nullptr;
    StringSplitter sp(source);
    int counter = 0;
    while (sp.canAdvance()) {
        sp.advance('+');
        ++counter;
    }
    EXPECT_EQ(counter, 0);
}

TEST(StringSplitter, returns_no_tokens_for_empty_string) {
    const char* source = "";
    StringSplitter sp(source);
    int counter = 0;
    while (sp.canAdvance()) {
        sp.advance('+');
        ++counter;
    }
    EXPECT_EQ(counter, 0);
}

TEST(StringSplitter, returns_whole_string_when_delimiter_not_found) {
    const char* source = "abc_def";
    StringSplitter sp(source);
    while (sp.canAdvance()) {
        auto sv = sp.advance('+');
        EXPECT_EQ(sv, "abc_def");
    }
}

TEST(StringSplitter, splits_string_with_one_delimiter) {
    const char* source = "abc def";
    StringSplitter sp(source);
    const char* expectations[] = { "abc", "def" };
    for (int i = 0; sp.canAdvance(); ++i) {
        auto sv = sp.advance(' ');
        EXPECT_EQ(sv, expectations[i]);
    }
}

TEST(StringSplitter, ignores_trailing_empty_token) {
    const char* source = "abcdef/";
    StringSplitter sp(source);
    const char* expectations[] = { "abcdef" };
    for (int i = 0; sp.canAdvance(); ++i) {
        auto sv = sp.advance('/');
        EXPECT_EQ(sv, expectations[i]);
    }
}

TEST(StringSplitter, splits_windows_path_by_backslash) {
    const char* source = "D:\\random\\path\\to\\my\\file\\";
    StringSplitter sp(source);
    const char* expectations[] = { "D:", "random", "path", "to", "my", "file" };
    for (int i = 0; sp.canAdvance(); ++i) {
        [[maybe_unused]] auto sv = sp.advance('\\');
        EXPECT_EQ(sv, expectations[i]);
        // printf("{%s}\n", expectations[i]);
    }
}

TEST(StringUtils_IsDigit, returns_true_for_ascii_digits) {
    EXPECT_TRUE(StringUtils::isDigit('0'));
    EXPECT_TRUE(StringUtils::isDigit('1'));
    EXPECT_TRUE(StringUtils::isDigit('2'));
    EXPECT_TRUE(StringUtils::isDigit('5'));
    EXPECT_TRUE(StringUtils::isDigit('8'));
    EXPECT_TRUE(StringUtils::isDigit('9'));
    EXPECT_FALSE(StringUtils::isDigit('9' + 1));
    EXPECT_FALSE(StringUtils::isDigit('A'));
    EXPECT_FALSE(StringUtils::isDigit('a'));
    EXPECT_FALSE(StringUtils::isDigit('+'));
    EXPECT_FALSE(StringUtils::isDigit('-'));
}

TEST(StringUtils_IsHex, returns_true_for_ascii_hex_characters) {
    EXPECT_TRUE(StringUtils::isHex('0'));
    EXPECT_TRUE(StringUtils::isHex('1'));
    EXPECT_TRUE(StringUtils::isHex('2'));
    EXPECT_TRUE(StringUtils::isHex('5'));
    EXPECT_TRUE(StringUtils::isHex('8'));
    EXPECT_TRUE(StringUtils::isHex('9'));
    EXPECT_TRUE(StringUtils::isHex('A'));
    EXPECT_TRUE(StringUtils::isHex('a'));
    EXPECT_TRUE(StringUtils::isHex('B'));
    EXPECT_TRUE(StringUtils::isHex('b'));
    EXPECT_TRUE(StringUtils::isHex('E'));
    EXPECT_TRUE(StringUtils::isHex('e'));
    EXPECT_TRUE(StringUtils::isHex('F'));
    EXPECT_TRUE(StringUtils::isHex('f'));
    EXPECT_FALSE(StringUtils::isHex('G'));
    EXPECT_FALSE(StringUtils::isHex('g'));
    EXPECT_FALSE(StringUtils::isHex('9' + 1));
    EXPECT_FALSE(StringUtils::isHex('+'));
    EXPECT_FALSE(StringUtils::isHex('-'));
}

TEST(StringUtils_HexToInt, converts_hex_character_to_integer) {
    EXPECT_EQ(StringUtils::hexToInt('0'), 0);
    EXPECT_EQ(StringUtils::hexToInt('2'), 2);
    EXPECT_EQ(StringUtils::hexToInt('5'), 5);
    EXPECT_EQ(StringUtils::hexToInt('8'), 8);
    EXPECT_EQ(StringUtils::hexToInt('9'), 9);
    EXPECT_EQ(StringUtils::hexToInt('f'), 15);
    EXPECT_EQ(StringUtils::hexToInt('F'), 15);
    EXPECT_EQ(StringUtils::hexToInt('a'), 10);
    EXPECT_EQ(StringUtils::hexToInt('A'), 10);
    EXPECT_EQ(StringUtils::hexToInt('c'), 12);
    EXPECT_EQ(StringUtils::hexToInt('E'), 14);
    EXPECT_EQ(StringUtils::hexToInt('*'), -1);
    EXPECT_EQ(StringUtils::hexToInt('-'), -1);
}

TEST(StringUtils_BasePath, returns_empty_when_path_has_no_separator) {
    constexpr std::string_view path{ "abcd.txt" };
    constexpr auto base_path = StringUtils::basePath(path, '/');
    EXPECT_EQ(base_path, "");
}

TEST(StringUtils_BasePath, returns_parent_for_single_level_path) {
    constexpr std::string_view path{ "C:/abcd.txt" };
    constexpr auto base_path = StringUtils::basePath(path, '/');
    EXPECT_EQ(base_path, "C:");
}

TEST(StringUtils_BasePath, returns_parent_for_multi_level_path) {
    constexpr std::string_view path{ "/dev/path/abcd.txt" };
    constexpr auto base_path = StringUtils::basePath(path, '/');
    EXPECT_EQ(base_path, "/dev/path");
}

TEST(StringUtils_FileName, returns_input_when_path_has_no_separator) {
    constexpr std::string_view path{ "ddd.txt" };
    constexpr auto name = StringUtils::fileName(path, '/');
    EXPECT_EQ(name, "ddd.txt");
}

TEST(StringUtils_FileName, returns_file_name_for_single_level_path) {
    constexpr std::string_view path{ "my/.git" };
    constexpr auto name = StringUtils::fileName(path, '/');
    EXPECT_EQ(name, ".git");
}

TEST(StringUtils_FileName, returns_file_name_for_multi_level_path) {
    constexpr std::string_view path{ "/dev/path/txt" };
    constexpr auto name = StringUtils::fileName(path, '/');
    EXPECT_EQ(name, "txt");
}

TEST(StringUtils_Extension, returns_empty_when_file_has_no_extension) {
    constexpr std::string_view path{ "/dev/path/txt" };
    constexpr auto name = StringUtils::extension(path);
    EXPECT_EQ(name, "");
}

TEST(StringUtils_Extension, returns_extension_for_file_with_one_extension) {
    constexpr std::string_view path{ "abc.txt" };
    constexpr auto name = StringUtils::extension(path);
    EXPECT_EQ(name, ".txt");
}

TEST(StringUtils_Extension, returns_last_extension_for_file_with_multiple_extensions) {
    constexpr std::string_view path{ "abc.my.format.scene" };
    constexpr auto name = StringUtils::extension(path);
    EXPECT_EQ(name, ".scene");
}

TEST(StringUtils_RemoveExtension, removes_extension_when_present) {
    auto file = StringUtils::removeExtension("abc.lua");
    EXPECT_EQ(file, "abc");
}

TEST(StringUtils_RemoveExtension, returns_input_when_extension_not_present) {
    auto file = StringUtils::removeExtension("abc");
    EXPECT_EQ(file, "abc");
}

}  // namespace cave::string
