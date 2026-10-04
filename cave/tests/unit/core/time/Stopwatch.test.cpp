#include "cave/core/time/Stopwatch.h"

namespace cave::time {

class TestClock {
public:
    static uint64_t Now() { return s_counter; }

    static void Tick(uint64_t p_value) { s_counter += p_value; }

    static void Reset() { s_counter = 0; }

private:
    inline static uint64_t s_counter{ 0 };
};

using TestStopwatch = StopwatchBase<uint64_t, TestClock>;

TEST(StopwatchBase, start_should_set_start_and_running) {
    TestClock::Reset();

    TestStopwatch sw;
    sw.start();

    EXPECT_EQ(sw.startPoint(), 0);
    EXPECT_EQ(sw.isRunning(), true);
}

TEST(StopwatchBase, elapsed_should_not_advance_when_not_running) {
    TestClock::Reset();

    TestStopwatch sw;
    // not started
    EXPECT_EQ(sw.elapsed(), 0);

    TestClock::Tick(10);
    // still not started => still 0
    EXPECT_EQ(sw.elapsed(), 0);

    // start then stop
    sw.start();
    TestClock::Tick(5);
    sw.stop();
    EXPECT_EQ(sw.elapsed(), 5);

    // time moves, but stopwatch stopped => elapsed stays the same
    TestClock::Tick(100);
    EXPECT_EQ(sw.elapsed(), 5);
    EXPECT_EQ(sw.isRunning(), false);
}

TEST(StopwatchBase, reset_should_clear_elapsed_and_stop) {
    TestClock::Reset();

    TestStopwatch sw;
    sw.start();
    TestClock::Tick(7);

    EXPECT_EQ(sw.elapsed(), 7);
    EXPECT_EQ(sw.isRunning(), true);

    sw.reset();

    EXPECT_EQ(sw.elapsed(), 0);
    EXPECT_EQ(sw.isRunning(), false);

    // After reset, time moves but still stopped
    TestClock::Tick(50);
    EXPECT_EQ(sw.elapsed(), 0);
}

TEST(StopwatchBase, multiple_start_stop_should_accumulate_elapsed) {
    TestClock::Reset();

    TestStopwatch sw;

    sw.start();
    TestClock::Tick(3);
    sw.stop();
    EXPECT_EQ(sw.elapsed(), 3);

    // idle time shouldn't count
    TestClock::Tick(10);
    EXPECT_EQ(sw.elapsed(), 3);

    sw.start();
    TestClock::Tick(4);
    sw.stop();
    EXPECT_EQ(sw.elapsed(), 7);
    EXPECT_EQ(sw.isRunning(), false);
}

TEST(StopwatchBase, start_while_running_should_restart_measurement) {
    TestClock::Reset();

    TestStopwatch sw;
    sw.start();
    TestClock::Tick(5);

    // Restart without stopping: current 5 is NOT committed to elapsed by design
    sw.start();
    EXPECT_EQ(sw.startPoint(), 5);
    EXPECT_EQ(sw.elapsed(), 0);  // elapsed still 0, because we restarted

    TestClock::Tick(2);
    EXPECT_EQ(sw.elapsed(), 2);

    sw.stop();
    EXPECT_EQ(sw.elapsed(), 2);
}

TEST(StopwatchBase, start_after_stop_should_keep_elapsed_and_continue_accumulating) {
    TestClock::Reset();

    TestStopwatch sw;
    sw.start();
    TestClock::Tick(6);
    sw.stop();
    EXPECT_EQ(sw.elapsed(), 6);

    // time moves while stopped
    TestClock::Tick(10);
    EXPECT_EQ(sw.elapsed(), 6);

    // start again continues accumulating from existing elapsed
    sw.start();
    EXPECT_EQ(sw.startPoint(), 16);

    TestClock::Tick(4);
    EXPECT_EQ(sw.elapsed(), 10);  // 6 + 4
    sw.stop();
    EXPECT_EQ(sw.elapsed(), 10);
}

TEST(StopwatchBase, stop_without_start_should_do_nothing) {
    TestClock::Reset();

    TestStopwatch sw;
    TestClock::Tick(8);

    sw.stop();
    EXPECT_EQ(sw.elapsed(), 0);
    EXPECT_EQ(sw.isRunning(), false);
}

TEST(StopwatchBase, restart_should_return_previous_elapsed_and_start_fresh) {
    TestClock::Reset();

    TestStopwatch sw;

    // First run: 0 -> 5
    sw.start();
    TestClock::Tick(5);
    EXPECT_EQ(sw.elapsed(), 5);

    const uint64_t prev = sw.restart();

    EXPECT_EQ(prev, 5);
    EXPECT_EQ(sw.isRunning(), true);
    EXPECT_EQ(sw.startPoint(), 5);
    EXPECT_EQ(sw.elapsed(), 0);

    // After restart, measure new segment: 5 -> 12
    TestClock::Tick(7);
    EXPECT_EQ(sw.elapsed(), 7);

    // Stop freezes it
    sw.stop();
    EXPECT_EQ(sw.isRunning(), false);
    EXPECT_EQ(sw.elapsed(), 7);

    // Further ticks shouldn't change elapsed
    TestClock::Tick(100);
    EXPECT_EQ(sw.elapsed(), 7);
}

}  // namespace cave::time
