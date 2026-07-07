#include "cave/core/time/CountdownTimer.h"

namespace cave {

TEST(CountdownTimer, DefaultConstructedIsFinished) {
    CountdownTimer timer;

    EXPECT_FALSE(timer.active());
    EXPECT_TRUE(timer.finished());
    EXPECT_EQ(timer.duration(), 0.0f);
    EXPECT_EQ(timer.remaining(), 0.0f);
}

TEST(CountdownTimer, ConstructedWithDurationDoesNotStartAutomatically) {
    CountdownTimer timer{ 2.0f };

    EXPECT_FALSE(timer.active());
    EXPECT_TRUE(timer.finished());
    EXPECT_EQ(timer.duration(), 2.0f);
    EXPECT_EQ(timer.remaining(), 0.0f);
}

TEST(CountdownTimer, StartUsesStoredDuration) {
    CountdownTimer timer{ 2.0f };

    timer.start();

    EXPECT_TRUE(timer.active());
    EXPECT_FALSE(timer.finished());
    EXPECT_EQ(timer.duration(), 2.0f);
    EXPECT_EQ(timer.remaining(), 2.0f);
}

TEST(CountdownTimer, StartWithDurationUpdatesDurationAndRemaining) {
    CountdownTimer timer{ 1.0f };

    timer.start(3.5f);

    EXPECT_TRUE(timer.active());
    EXPECT_EQ(timer.duration(), 3.5f);
    EXPECT_EQ(timer.remaining(), 3.5f);
}

TEST(CountdownTimer, TickReducesRemainingTime) {
    CountdownTimer timer{ 2.0f };
    timer.start();

    timer.tick(0.5f);

    EXPECT_TRUE(timer.active());
    EXPECT_FLOAT_EQ(timer.remaining(), 1.5f);
}

TEST(CountdownTimer, TickClampsAtZero) {
    CountdownTimer timer{ 2.0f };
    timer.start();

    timer.tick(5.0f);

    EXPECT_FALSE(timer.active());
    EXPECT_TRUE(timer.finished());
    EXPECT_EQ(timer.remaining(), 0.0f);
}

TEST(CountdownTimer, StopFinishesTimer) {
    CountdownTimer timer{ 2.0f };
    timer.start();

    timer.stop();

    EXPECT_FALSE(timer.active());
    EXPECT_TRUE(timer.finished());
    EXPECT_EQ(timer.remaining(), 0.0f);
    EXPECT_EQ(timer.duration(), 2.0f);
}

TEST(CountdownTimer, ResetRestartsUsingStoredDuration) {
    CountdownTimer timer{ 2.0f };
    timer.start();
    timer.tick(0.75f);

    timer.reset();

    EXPECT_TRUE(timer.active());
    EXPECT_EQ(timer.remaining(), 2.0f);
}

TEST(CountdownTimer, Progress01StartsAtZeroAfterStart) {
    CountdownTimer timer{ 4.0f };
    timer.start();

    EXPECT_FLOAT_EQ(timer.progress01(), 0.0f);
}

TEST(CountdownTimer, Progress01ReachesOneWhenFinished) {
    CountdownTimer timer{ 4.0f };
    timer.start();

    timer.tick(4.0f);

    EXPECT_FLOAT_EQ(timer.progress01(), 1.0f);
}

TEST(CountdownTimer, Progress01TracksElapsedFraction) {
    CountdownTimer timer{ 4.0f };
    timer.start();

    timer.tick(1.0f);

    EXPECT_FLOAT_EQ(timer.progress01(), 0.25f);
}

TEST(CountdownTimer, Progress01ForZeroDurationIsOne) {
    CountdownTimer timer;

    EXPECT_FLOAT_EQ(timer.progress01(), 1.0f);
}

}  // namespace cave