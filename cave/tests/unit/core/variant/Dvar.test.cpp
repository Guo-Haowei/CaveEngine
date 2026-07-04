#include "engine/private/core/variant/Dvar.h"

namespace cave::variant {

using namespace cave::math;

#define DEFINE_DVAR
#include "TestDvars.h"

void RegisterTestDvars() {
    static bool s_registered = false;
    if (!s_registered) {
#define REGISTER_DVAR
#include "TestDvars.h"
#undef REGISTER_DVAR
    }
    s_registered = true;
}

class DvarTest : public ::testing::Test {
protected:
    void SetUp() override {
        RegisterTestDvars();
    }
};

TEST_F(DvarTest, dvar_int) {
    auto value = DVAR_GET_INT(test_int);
    EXPECT_EQ(value, 100);
    DVAR_SET_INT(test_int, 200);
    value = DVAR_GET_INT(test_int);
    EXPECT_EQ(value, 200);
}

TEST_F(DvarTest, dvar_float) {
    auto value = DVAR_GET_FLOAT(test_float);
    EXPECT_EQ(value, 2.3f);
    DVAR_SET_FLOAT(test_float, 1.2f);
    value = DVAR_GET_FLOAT(test_float);
    EXPECT_EQ(value, 1.2f);
}

TEST_F(DvarTest, dvar_string) {
    auto value = DVAR_GET_STRING(test_string);
    EXPECT_EQ(value, "abc");
    DVAR_SET_STRING(test_string, std::string_view("bcd"));
    value = DVAR_GET_STRING(test_string);
    EXPECT_EQ(value, "bcd");
}

TEST_F(DvarTest, dvar_vec2f) {
    auto value = DVAR_GET_VEC2(test_vec2);
    EXPECT_EQ(value, Vec2f(1, 2));
    DVAR_SET_VEC2(test_vec2, 7.0f, 8.0f);
    value = DVAR_GET_VEC2(test_vec2);
    EXPECT_EQ(value, Vec2f(7, 8));
}

TEST_F(DvarTest, dvar_vec3f) {
    auto value = DVAR_GET_VEC3(test_vec3);
    EXPECT_EQ(value, Vec3f(1, 2, 3));
    DVAR_SET_VEC3(test_vec3, 7.0f, 8.0f, 9.0f);
    value = DVAR_GET_VEC3(test_vec3);
    EXPECT_EQ(value, Vec3f(7, 8, 9));
}

TEST_F(DvarTest, dvar_vec4f) {
    auto value = DVAR_GET_VEC4(test_vec4);
    EXPECT_EQ(value, Vec4f(1, 2, 3, 4));
    DVAR_SET_VEC4(test_vec4, 7.0f, 8.0f, 9.0f, 10.0f);
    value = DVAR_GET_VEC4(test_vec4);
    EXPECT_EQ(value, Vec4f(7, 8, 9, 10));
}

TEST_F(DvarTest, dvar_vec2i) {
    auto value = DVAR_GET_IVEC2(test_ivec2);
    EXPECT_EQ(value, Vec2i(1, 2));
    DVAR_SET_IVEC2(test_ivec2, 7, 8);
    value = DVAR_GET_IVEC2(test_ivec2);
    EXPECT_EQ(value, Vec2i(7, 8));
}

TEST_F(DvarTest, dvar_vec3i) {
    auto value = DVAR_GET_IVEC3(test_ivec3);
    EXPECT_EQ(value, Vec3i(1, 2, 3));
    DVAR_SET_IVEC3(test_ivec3, 7, 8, 9);
    value = DVAR_GET_IVEC3(test_ivec3);
    EXPECT_EQ(value, Vec3i(7, 8, 9));
}

TEST_F(DvarTest, dvar_vec4i) {
    auto value = DVAR_GET_IVEC4(test_ivec4);
    EXPECT_EQ(value, Vec4i(1, 2, 3, 4));
    DVAR_SET_IVEC4(test_ivec4, 7, 8, 9, 10);
    value = DVAR_GET_IVEC4(test_ivec4);
    EXPECT_EQ(value, Vec4i(7, 8, 9, 10));
}

}  // namespace cave::variant
