#include "cave/core/string/FixedString.h"

namespace cave::string {

TEST(FixedString, default_constructor) {
    FixedString<32> str;

    EXPECT_EQ(str.capacity(), 31);
    EXPECT_EQ(str.size(), 0);
    EXPECT_TRUE(str.empty());
    EXPECT_EQ(str.view(), std::string_view());
    EXPECT_STREQ(str.c_str(), "");
}

TEST(FixedString, construct_from_string_view) {
    FixedString<32> str(std::string_view("hello"));

    EXPECT_FALSE(str.empty());
    EXPECT_EQ(str.size(), 5);
    EXPECT_EQ(str.view(), "hello");
    EXPECT_STREQ(str.c_str(), "hello");
}

TEST(FixedString, construct_from_c_string) {
    FixedString<32> str("abc");

    EXPECT_EQ(str.size(), 3);
    EXPECT_EQ(str.view(), "abc");
    EXPECT_STREQ(str.c_str(), "abc");
}

TEST(FixedString, assign_string_view) {
    FixedString<32> str;
    str.assign("world");

    EXPECT_EQ(str.size(), 5);
    EXPECT_EQ(str.view(), "world");
    EXPECT_STREQ(str.c_str(), "world");
}

TEST(FixedString, operator_assign_string_view) {
    FixedString<32> str;
    str = "test";

    EXPECT_EQ(str.size(), 4);
    EXPECT_EQ(str.view(), "test");
    EXPECT_STREQ(str.c_str(), "test");
}

TEST(FixedString, clear) {
    FixedString<32> str("abc");
    str.clear();

    EXPECT_TRUE(str.empty());
    EXPECT_EQ(str.size(), 0);
    EXPECT_STREQ(str.c_str(), "");
}

TEST(FixedString, append) {
    FixedString<32> str("hello");
    str.append(" world");

    EXPECT_EQ(str.view(), "hello world");
    EXPECT_EQ(str.size(), 11);
    EXPECT_STREQ(str.c_str(), "hello world");
}

TEST(FixedString, operator_plus_equal) {
    FixedString<32> str("a");
    str += "bc";

    EXPECT_EQ(str.view(), "abc");
    EXPECT_EQ(str.size(), 3);
}

TEST(FixedString, push_back) {
    FixedString<8> str("ab");
    str.push_back('c');

    EXPECT_EQ(str.view(), "abc");
    EXPECT_EQ(str.size(), 3);
    EXPECT_STREQ(str.c_str(), "abc");
}

TEST(FixedString, push_back_overflow) {
    FixedString<4> str("abc");  // capacity = 3
    str.push_back('x');

    EXPECT_EQ(str.view(), "abc");
    EXPECT_EQ(str.size(), 3);
    EXPECT_STREQ(str.c_str(), "abc");
}

TEST(FixedString, truncation_on_assign) {
    FixedString<6> str;  // capacity = 5
    str.assign("123456789");

    EXPECT_EQ(str.size(), 5);
    EXPECT_EQ(str.view(), "12345");
    EXPECT_STREQ(str.c_str(), "12345");
}

TEST(FixedString, truncation_on_append) {
    FixedString<6> str("12");  // capacity = 5
    str.append("3456789");

    EXPECT_EQ(str.size(), 5);
    EXPECT_EQ(str.view(), "12345");
    EXPECT_STREQ(str.c_str(), "12345");
}

TEST(FixedString, operator_brackets) {
    FixedString<16> str("abcd");

    EXPECT_EQ(str[0], 'a');
    EXPECT_EQ(str[3], 'd');

    str[1] = 'x';
    EXPECT_EQ(str.view(), "axcd");
}

TEST(FixedString, front_back) {
    FixedString<16> str("abcd");

    EXPECT_EQ(str.front(), 'a');
    EXPECT_EQ(str.back(), 'd');
}

TEST(FixedString, compare_equal) {
    FixedString<16> str("hello");

    EXPECT_EQ(str.compare("hello"), 0);
    EXPECT_TRUE(str == std::string_view("hello"));
    EXPECT_FALSE(str != std::string_view("hello"));
}

TEST(FixedString, compare_not_equal) {
    FixedString<16> str("hello");

    EXPECT_NE(str.compare("world"), 0);
    EXPECT_FALSE(str == std::string_view("world"));
    EXPECT_TRUE(str != std::string_view("world"));
}

TEST(FixedString, equality_fixed_string) {
    FixedString<16> a("abc");
    FixedString<16> b("abc");
    FixedString<16> c("abcd");

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}

TEST(FixedString, starts_with) {
    FixedString<32> str("hello_world");

    EXPECT_TRUE(str.starts_with("hello"));
    EXPECT_FALSE(str.starts_with("world"));
}

TEST(FixedString, ends_with) {
    FixedString<32> str("hello_world");

    EXPECT_TRUE(str.ends_with("world"));
    EXPECT_FALSE(str.ends_with("hello"));
}

TEST(FixedString, contains) {
    FixedString<32> str("hello_world");

    EXPECT_TRUE(str.contains("lo_w"));
    EXPECT_FALSE(str.contains("abc"));
}

TEST(FixedString, string_view_conversion) {
    FixedString<32> str("abc");

    std::string_view sv = str;
    EXPECT_EQ(sv, "abc");
}

TEST(FixedString, data_is_null_terminated) {
    FixedString<6> str("12345");  // capacity = 5

    EXPECT_EQ(str.size(), 5);
    EXPECT_EQ(str.c_str()[5], '\0');
}

TEST(FixedString, append_preserves_null_termination) {
    FixedString<6> str("12");  // capacity = 5
    str.append("345");

    EXPECT_EQ(str.size(), 5);
    EXPECT_EQ(str.view(), "12345");
    EXPECT_EQ(str.c_str()[5], '\0');
}

}  // namespace cave::string
