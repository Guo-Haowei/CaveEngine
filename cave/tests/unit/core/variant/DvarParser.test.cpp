#include "engine/private/core/variant/DvarParser.h"

namespace cave::variant {

#include "TestDvars.h"

using namespace cave::math;

extern void RegisterTestDvars();

using Commands = std::vector<std::string_view>;

class DvarParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        RegisterTestDvars();
    }
};

TEST_F(DvarParserTest, invalid_command) {
    Commands commands = { "+abc" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_FALSE(ok);
    EXPECT_EQ(parser.error(), "unknown command '+abc'");
}

TEST_F(DvarParserTest, invalid_dvar_name) {
    Commands commands = { "+set", "test_int1" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_FALSE(ok);
    EXPECT_EQ(parser.error(), "dvar 'test_int1' not found");
}

TEST_F(DvarParserTest, unexpected_eof) {
    Commands commands = { "+set", "test_int" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_FALSE(ok);
    EXPECT_EQ(parser.error(), "invalid arguments: test_int");
}

TEST_F(DvarParserTest, set_int) {
    Commands commands = { "+set", "test_int", "1001" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_INT(test_int), 1001);
}

TEST_F(DvarParserTest, set_float) {
    Commands commands = { "+set", "test_float", "1001.1" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_FLOAT(test_float), 1001.1f);
}

TEST_F(DvarParserTest, set_string) {
    Commands commands = { "+set", "test_string", "1001.1" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_STRING(test_string), "1001.1");
}

TEST_F(DvarParserTest, set_vec2) {
    Commands commands = { "+set", "test_vec2", "6", "7" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_VEC2(test_vec2), Vec2f(6, 7));
}

TEST_F(DvarParserTest, set_vec3) {
    Commands commands = { "+set", "test_vec3", "6.0", "7.0", "8.0" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_VEC3(test_vec3), Vec3f(6, 7, 8));
}

TEST_F(DvarParserTest, set_vec4) {
    Commands commands = { "+set", "test_vec4", "6", "7", "8", "9" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_VEC4(test_vec4), Vec4f(6, 7, 8, 9));
}

TEST_F(DvarParserTest, set_ivec2) {
    Commands commands = { "+set", "test_ivec2", "6", "7" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_IVEC2(test_ivec2), Vec2i(6, 7));
}

TEST_F(DvarParserTest, set_ivec3) {
    Commands commands = { "+set", "test_ivec3", "6", "7", "8" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_IVEC3(test_ivec3), Vec3i(6, 7, 8));
}

TEST_F(DvarParserTest, set_ivec4) {
    Commands commands = { "+set", "test_ivec4", "6", "7", "8", "9" };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_IVEC4(test_ivec4), Vec4i(6, 7, 8, 9));
}

TEST_F(DvarParserTest, multiple_set_success) {
    Commands commands = {
        // clang-format off
        "+set", "test_ivec4", "7", "8", "9", "10",
        "+set", "test_int", "1002",
        // clang-format on
    };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_IVEC4(test_ivec4), Vec4i(7, 8, 9, 10));
    EXPECT_EQ(DVAR_GET_INT(test_int), 1002);
}

TEST_F(DvarParserTest, multiple_set_fail) {
    Commands commands = {
        // clang-format off
        "+set", "test_ivec4", "7", "8", "9", "10",
        "+set", "test_int", "1002",
        "+set", "test_vec4", "1",
        // clang-format on
    };

    DvarParser parser(commands);
    bool ok = parser.parse();
    EXPECT_FALSE(ok);
    EXPECT_EQ(parser.error(), "invalid arguments: test_vec4 1");
}

}  // namespace cave::variant
