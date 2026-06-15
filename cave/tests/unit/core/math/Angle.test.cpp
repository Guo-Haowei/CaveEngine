#include "cave/core/math/Angle.h"

namespace cave::math {

namespace {

constexpr float kEpsilon = 1e-5f;

}  // namespace

TEST(Angle, DegreeConstructsFromFloatAndDouble) {
    constexpr Degree a{ 90.0f };
    constexpr Degree b{ 180.0 };

    EXPECT_FLOAT_EQ(a.degrees(), 90.0f);
    EXPECT_FLOAT_EQ(b.degrees(), 180.0f);
}

TEST(Angle, RadianConstructsFromFloat) {
    constexpr Radian r{ 1.5f };

    EXPECT_FLOAT_EQ(r.radians(), 1.5f);
}

TEST(Angle, DegreeConvertsToRadians) {
    EXPECT_NEAR(Degree{ 0.0f }.radians(), 0.0f, kEpsilon);
    EXPECT_NEAR(Degree{ 90.0f }.radians(), pi<float>() * 0.5f, kEpsilon);
    EXPECT_NEAR(Degree{ 180.0f }.radians(), pi<float>(), kEpsilon);
    EXPECT_NEAR(Degree{ 360.0f }.radians(), pi<float>() * 2.0f, kEpsilon);
}

TEST(Angle, RadianConvertsToDegrees) {
    EXPECT_NEAR(Radian{ 0.0f }.degrees(), 0.0f, kEpsilon);
    EXPECT_NEAR(Radian{ pi<float>() * 0.5f }.degrees(), 90.0f, kEpsilon);
    EXPECT_NEAR(Radian{ pi<float>() }.degrees(), 180.0f, kEpsilon);
    EXPECT_NEAR(Radian{ pi<float>() * 2.0f }.degrees(), 360.0f, kEpsilon);
}

TEST(Angle, DegreeArithmetic) {
    Degree angle{ 90.0f };

    EXPECT_FLOAT_EQ((angle * 2.0f).degrees(), 180.0f);
    EXPECT_FLOAT_EQ((angle / 2.0f).degrees(), 45.0f);

    angle += Degree{ 30.0f };
    EXPECT_FLOAT_EQ(angle.degrees(), 120.0f);

    angle -= Degree{ 20.0f };
    EXPECT_FLOAT_EQ(angle.degrees(), 100.0f);

    angle *= 2.0f;
    EXPECT_FLOAT_EQ(angle.degrees(), 200.0f);

    angle /= 4.0f;
    EXPECT_FLOAT_EQ(angle.degrees(), 50.0f);
}

TEST(Angle, RadianArithmetic) {
    Radian angle{ 1.0f };

    EXPECT_FLOAT_EQ((angle * 2.0f).radians(), 2.0f);
    EXPECT_FLOAT_EQ((angle / 2.0f).radians(), 0.5f);

    angle += Radian{ 0.5f };
    EXPECT_FLOAT_EQ(angle.radians(), 1.5f);

    angle -= Radian{ 0.25f };
    EXPECT_FLOAT_EQ(angle.radians(), 1.25f);

    angle *= 2.0f;
    EXPECT_FLOAT_EQ(angle.radians(), 2.5f);

    angle /= 5.0f;
    EXPECT_FLOAT_EQ(angle.radians(), 0.5f);
}

TEST(Angle, RadianCanAddAndSubtractDegree) {
    Radian angle{ 0.0f };

    angle += Degree{ 180.0f };
    EXPECT_NEAR(angle.radians(), pi<float>(), kEpsilon);

    angle -= Degree{ 90.0f };
    EXPECT_NEAR(angle.radians(), pi<float>() * 0.5f, kEpsilon);
}

TEST(Angle, DegreeComparison) {
    EXPECT_EQ(Degree{ 90.0f }, Degree{ 90.0f });
    EXPECT_NE(Degree{ 90.0f }, Degree{ 180.0f });

    EXPECT_LT(Degree{ 45.0f }, Degree{ 90.0f });
    EXPECT_LE(Degree{ 90.0f }, Degree{ 90.0f });
    EXPECT_GT(Degree{ 180.0f }, Degree{ 90.0f });
    EXPECT_GE(Degree{ 180.0f }, Degree{ 180.0f });
}

TEST(Angle, RadianComparison) {
    EXPECT_EQ(Radian{ 1.0f }, Radian{ 1.0f });
    EXPECT_NE(Radian{ 1.0f }, Radian{ 2.0f });

    EXPECT_LT(Radian{ 0.5f }, Radian{ 1.0f });
    EXPECT_LE(Radian{ 1.0f }, Radian{ 1.0f });
    EXPECT_GT(Radian{ 2.0f }, Radian{ 1.0f });
    EXPECT_GE(Radian{ 2.0f }, Radian{ 2.0f });
}

TEST(Angle, DegreeClamp) {
    Degree angle{ 120.0f };

    angle.clamp(0.0f, 90.0f);
    EXPECT_FLOAT_EQ(angle.degrees(), 90.0f);

    angle = Degree{ -20.0f };
    angle.clamp(0.0f, 90.0f);
    EXPECT_FLOAT_EQ(angle.degrees(), 0.0f);

    angle = Degree{ 45.0f };
    angle.clamp(0.0f, 90.0f);
    EXPECT_FLOAT_EQ(angle.degrees(), 45.0f);
}

TEST(Angle, RadianClamp) {
    Radian angle{ 2.0f };

    angle.clamp(0.0f, 1.0f);
    EXPECT_FLOAT_EQ(angle.radians(), 1.0f);

    angle = Radian{ -1.0f };
    angle.clamp(0.0f, 1.0f);
    EXPECT_FLOAT_EQ(angle.radians(), 0.0f);

    angle = Radian{ 0.5f };
    angle.clamp(0.0f, 1.0f);
    EXPECT_FLOAT_EQ(angle.radians(), 0.5f);
}

TEST(Angle, DegreeTrig) {
    EXPECT_NEAR(Degree{ 0.0f }.sin(), 0.0f, kEpsilon);
    EXPECT_NEAR(Degree{ 90.0f }.sin(), 1.0f, kEpsilon);
    EXPECT_NEAR(Degree{ 180.0f }.sin(), 0.0f, kEpsilon);

    EXPECT_NEAR(Degree{ 0.0f }.cos(), 1.0f, kEpsilon);
    EXPECT_NEAR(Degree{ 90.0f }.cos(), 0.0f, kEpsilon);
    EXPECT_NEAR(Degree{ 180.0f }.cos(), -1.0f, kEpsilon);

    EXPECT_NEAR(Degree{ 45.0f }.tan(), 1.0f, kEpsilon);
}

TEST(Angle, RadianTrig) {
    EXPECT_NEAR(Radian{ 0.0f }.sin(), 0.0f, kEpsilon);
    EXPECT_NEAR(Radian{ pi<float>() * 0.5f }.sin(), 1.0f, kEpsilon);
    EXPECT_NEAR(Radian{ pi<float>() }.sin(), 0.0f, kEpsilon);

    EXPECT_NEAR(Radian{ 0.0f }.cos(), 1.0f, kEpsilon);
    EXPECT_NEAR(Radian{ pi<float>() * 0.5f }.cos(), 0.0f, kEpsilon);
    EXPECT_NEAR(Radian{ pi<float>() }.cos(), -1.0f, kEpsilon);

    EXPECT_NEAR(Radian{ pi<float>() * 0.25f }.tan(), 1.0f, kEpsilon);
}

TEST(Angle, DegreeUnaryMinus) {
    Degree angle{ 90.0f };
    Degree negative = -angle;

    EXPECT_FLOAT_EQ(negative.degrees(), -90.0f);
}

}  // namespace cave::math
