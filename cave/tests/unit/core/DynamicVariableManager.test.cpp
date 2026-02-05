#include "engine/private/core/os/os.h"
#include "engine/private/runtime/dvar/DvarParser.h"

namespace cave {

#include "TestDvars.h"

using namespace cave::math;

extern void register_test_dvars();

using Commands = std::vector<std::string_view>;

TEST(DvarPaser, invalid_command) {
    register_test_dvars();

    Commands commands = { "+abc" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_FALSE(ok);
    EXPECT_EQ(parser.GetError(), "unknown command '+abc'");
}

TEST(DvarPaser, invalid_dvar_name) {
    register_test_dvars();

    Commands commands = { "+set", "test_int1" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_FALSE(ok);
    EXPECT_EQ(parser.GetError(), "dvar 'test_int1' not found");
}

TEST(DvarPaser, unexpected_eof) {
    register_test_dvars();

    Commands commands = { "+set", "test_int" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_FALSE(ok);
    EXPECT_EQ(parser.GetError(), "invalid arguments: test_int");
}

TEST(DvarPaser, set_int) {
    register_test_dvars();

    Commands commands = { "+set", "test_int", "1001" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_INT(test_int), 1001);
}

TEST(DvarPaser, set_float) {
    register_test_dvars();

    Commands commands = { "+set", "test_float", "1001.1" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_FLOAT(test_float), 1001.1f);
}

TEST(DvarPaser, set_string) {
    register_test_dvars();

    Commands commands = { "+set", "test_string", "1001.1" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_STRING(test_string), "1001.1");
}

TEST(DvarPaser, set_vec2) {
    register_test_dvars();

    Commands commands = { "+set", "test_vec2", "6", "7" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_VEC2(test_vec2), Vector2f(6, 7));
}

TEST(DvarPaser, set_vec3) {
    register_test_dvars();

    Commands commands = { "+set", "test_vec3", "6.0", "7.0", "8.0" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_VEC3(test_vec3), Vector3f(6, 7, 8));
}

TEST(DvarPaser, set_vec4) {
    register_test_dvars();

    Commands commands = { "+set", "test_vec4", "6", "7", "8", "9" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_VEC4(test_vec4), Vector4f(6, 7, 8, 9));
}

TEST(DvarPaser, set_ivec2) {
    register_test_dvars();

    Commands commands = { "+set", "test_ivec2", "6", "7" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_IVEC2(test_ivec2), Vector2i(6, 7));
}

TEST(DvarPaser, set_ivec3) {
    register_test_dvars();

    Commands commands = { "+set", "test_ivec3", "6", "7", "8" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_IVEC3(test_ivec3), Vector3i(6, 7, 8));
}

TEST(DvarPaser, set_ivec4) {
    register_test_dvars();

    Commands commands = { "+set", "test_ivec4", "6", "7", "8", "9" };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_IVEC4(test_ivec4), Vector4i(6, 7, 8, 9));
}

TEST(DvarPaser, multiple_set_success) {
    register_test_dvars();

    Commands commands = {
        // clang-format off
        "+set", "test_ivec4", "7", "8", "9", "10",
        "+set", "test_int", "1002",
        // clang-format on
    };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_TRUE(ok);
    EXPECT_EQ(DVAR_GET_IVEC4(test_ivec4), Vector4i(7, 8, 9, 10));
    EXPECT_EQ(DVAR_GET_INT(test_int), 1002);
}

TEST(DvarPaser, multiple_set_fail) {
    register_test_dvars();

    Commands commands = {
        // clang-format off
        "+set", "test_ivec4", "7", "8", "9", "10",
        "+set", "test_int", "1002",
        "+set", "test_vec4", "1",
        // clang-format on
    };

    DvarParser parser(commands);
    bool ok = parser.Parse();
    EXPECT_FALSE(ok);
    EXPECT_EQ(parser.GetError(), "invalid arguments: test_vec4 1");
}

}  // namespace cave
