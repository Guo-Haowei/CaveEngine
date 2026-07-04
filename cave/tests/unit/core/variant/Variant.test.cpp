#include "cave/core/variant/Variant.h"

namespace cave {

using namespace ::cave::math;

TEST(Variant, default_constructed_variant_is_invalid) {
    Variant value;

    EXPECT_EQ(value.type(), VariantType::Invalid);
    EXPECT_FALSE(value.isValid());
    EXPECT_FALSE(value.isNumeric());

    EXPECT_FALSE(value.asBool());
    EXPECT_EQ(value.asInt(), 0);
    EXPECT_FLOAT_EQ(value.asFloat(), 0.0f);
    EXPECT_EQ(value.asString(), std::string_view{});
    EXPECT_EQ(value.asVec2f(), Vec2f::Zero);
    EXPECT_EQ(value.asVec3f(), Vec3f::Zero);
    EXPECT_EQ(value.asVec4f(), Vec4f::Zero);
    EXPECT_EQ(value.asVec2i(), Vec2i::Zero);
    EXPECT_EQ(value.asVec3i(), Vec3i::Zero);
    EXPECT_EQ(value.asVec4i(), Vec4i::Zero);
}

TEST(Variant, construct_from_bool_stores_as_integer_like_value) {
    Variant false_value(false);
    Variant true_value(true);

    // If you add VariantType::Bool, change these to VariantType::Bool.
    EXPECT_EQ(false_value.type(), VariantType::Int);
    EXPECT_EQ(true_value.type(), VariantType::Int);

    EXPECT_FALSE(false_value.asBool(true));
    EXPECT_TRUE(true_value.asBool(false));

    EXPECT_EQ(false_value.asInt(-1), 0);
    EXPECT_EQ(true_value.asInt(-1), 1);

    EXPECT_FLOAT_EQ(false_value.asFloat(-1.0f), 0.0f);
    EXPECT_FLOAT_EQ(true_value.asFloat(-1.0f), 1.0f);
}

TEST(Variant, construct_from_int) {
    Variant value(42);

    EXPECT_EQ(value.type(), VariantType::Int);
    EXPECT_TRUE(value.isValid());
    EXPECT_TRUE(value.isNumeric());

    EXPECT_TRUE(value.asBool(false));
    EXPECT_EQ(value.asInt(), 42);
    EXPECT_FLOAT_EQ(value.asFloat(), 42.0f);

    EXPECT_EQ(value.asString("fallback"), "fallback");
    EXPECT_EQ(value.asVec2f(Vec2f(1.0f, 2.0f)), Vec2f(1.0f, 2.0f));
}

TEST(Variant, construct_from_zero_int_as_bool_is_false) {
    Variant value(0);

    EXPECT_EQ(value.type(), VariantType::Int);
    EXPECT_FALSE(value.asBool(true));
    EXPECT_EQ(value.asInt(-1), 0);
    EXPECT_FLOAT_EQ(value.asFloat(-1.0f), 0.0f);
}

TEST(Variant, construct_from_float) {
    Variant value(3.5f);

    EXPECT_EQ(value.type(), VariantType::Float);
    EXPECT_TRUE(value.isValid());
    EXPECT_TRUE(value.isNumeric());

    EXPECT_TRUE(value.asBool(false));
    EXPECT_EQ(value.asInt(), 3);
    EXPECT_FLOAT_EQ(value.asFloat(), 3.5f);

    EXPECT_EQ(value.asString("fallback"), "fallback");
}

TEST(Variant, construct_from_zero_float_as_bool_is_false) {
    Variant value(0.0f);

    EXPECT_EQ(value.type(), VariantType::Float);
    EXPECT_FALSE(value.asBool(true));
    EXPECT_EQ(value.asInt(-1), 0);
    EXPECT_FLOAT_EQ(value.asFloat(-1.0f), 0.0f);
}

TEST(Variant, construct_from_c_string) {
    Variant value("hello");

    EXPECT_EQ(value.type(), VariantType::String);
    EXPECT_TRUE(value.isValid());
    EXPECT_FALSE(value.isNumeric());

    EXPECT_EQ(value.asString(), "hello");

    EXPECT_FALSE(value.asBool(false));
    EXPECT_EQ(value.asInt(123), 123);
    EXPECT_FLOAT_EQ(value.asFloat(4.0f), 4.0f);
}

TEST(Variant, construct_from_string_view) {
    std::string_view text = "hello_view";
    Variant value(text);

    EXPECT_EQ(value.type(), VariantType::String);
    EXPECT_EQ(value.asString(), "hello_view");
}

TEST(Variant, construct_from_string_copies_value) {
    std::string text = "original";
    Variant value(text);

    text = "changed";

    EXPECT_EQ(value.type(), VariantType::String);
    EXPECT_EQ(value.asString(), "original");
}

TEST(Variant, construct_from_moved_string) {
    std::string text = "moved";
    Variant value(std::move(text));

    EXPECT_EQ(value.type(), VariantType::String);
    EXPECT_EQ(value.asString(), "moved");
}

TEST(Variant, construct_from_vec2f) {
    Variant value(Vec2f(1.0f, 2.0f));

    EXPECT_EQ(value.type(), VariantType::Vec2f);
    EXPECT_TRUE(value.isValid());
    EXPECT_FALSE(value.isNumeric());

    EXPECT_EQ(value.asVec2f(), Vec2f(1.0f, 2.0f));
    EXPECT_EQ(value.asVec3f(), Vec3f(1.0f, 2.0f, 0.0f));
    EXPECT_EQ(value.asVec4f(), Vec4f(1.0f, 2.0f, 0.0f, 0.0f));

    EXPECT_EQ(value.asInt(7), 7);
    EXPECT_FLOAT_EQ(value.asFloat(8.0f), 8.0f);
}

TEST(Variant, construct_from_vec3f) {
    Variant value(Vec3f(1.0f, 2.0f, 3.0f));

    EXPECT_EQ(value.type(), VariantType::Vec3f);

    EXPECT_EQ(value.asVec2f(), Vec2f(1.0f, 2.0f));
    EXPECT_EQ(value.asVec3f(), Vec3f(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(value.asVec4f(), Vec4f(1.0f, 2.0f, 3.0f, 0.0f));
}

TEST(Variant, construct_from_vec4f) {
    Variant value(Vec4f(1.0f, 2.0f, 3.0f, 4.0f));

    EXPECT_EQ(value.type(), VariantType::Vec4f);

    EXPECT_EQ(value.asVec2f(), Vec2f(1.0f, 2.0f));
    EXPECT_EQ(value.asVec3f(), Vec3f(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(value.asVec4f(), Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(Variant, construct_from_vec2i) {
    Variant value(Vec2i(1, 2));

    EXPECT_EQ(value.type(), VariantType::Vec2i);

    EXPECT_EQ(value.asVec2i(), Vec2i(1, 2));
    EXPECT_EQ(value.asVec3i(), Vec3i(1, 2, 0));
    EXPECT_EQ(value.asVec4i(), Vec4i(1, 2, 0, 0));
}

TEST(Variant, construct_from_vec3i) {
    Variant value(Vec3i(1, 2, 3));

    EXPECT_EQ(value.type(), VariantType::Vec3i);

    EXPECT_EQ(value.asVec2i(), Vec2i(1, 2));
    EXPECT_EQ(value.asVec3i(), Vec3i(1, 2, 3));
    EXPECT_EQ(value.asVec4i(), Vec4i(1, 2, 3, 0));
}

TEST(Variant, construct_from_vec4i) {
    Variant value(Vec4i(1, 2, 3, 4));

    EXPECT_EQ(value.type(), VariantType::Vec4i);

    EXPECT_EQ(value.asVec2i(), Vec2i(1, 2));
    EXPECT_EQ(value.asVec3i(), Vec3i(1, 2, 3));
    EXPECT_EQ(value.asVec4i(), Vec4i(1, 2, 3, 4));
}

TEST(Variant, wrong_type_returns_fallback) {
    Variant string_value("hello");
    Variant vec_value(Vec3f(1.0f, 2.0f, 3.0f));

    EXPECT_EQ(string_value.asInt(99), 99);
    EXPECT_FLOAT_EQ(string_value.asFloat(9.5f), 9.5f);
    EXPECT_EQ(string_value.asVec2f(Vec2f(7.0f, 8.0f)), Vec2f(7.0f, 8.0f));

    EXPECT_EQ(vec_value.asString("fallback"), "fallback");
    EXPECT_EQ(vec_value.asInt(123), 123);
}

TEST(Variant, copy_variant_preserves_value) {
    Variant original("hello");
    Variant copy = original;

    EXPECT_EQ(copy.type(), VariantType::String);
    EXPECT_EQ(copy.asString(), "hello");
}

TEST(Variant, assign_variant_preserves_value) {
    Variant value(123);
    value = Variant("hello");

    EXPECT_EQ(value.type(), VariantType::String);
    EXPECT_EQ(value.asString(), "hello");
}

TEST(Variant, variant_map_stores_values) {
    VariantMap map;

    map["health"] = Variant(100);
    map["speed"] = Variant(3.5f);
    map["name"] = Variant("player");

    EXPECT_EQ(map["health"].asInt(), 100);
    EXPECT_FLOAT_EQ(map["speed"].asFloat(), 3.5f);
    EXPECT_EQ(map["name"].asString(), "player");
}

}  // namespace cave