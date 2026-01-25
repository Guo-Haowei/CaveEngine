#include "cave/runtime/gameplay/GameSession.h"

namespace cave::gameplay {

struct Counters {
    int enter_count = 0;
    int exit_count = 0;
    int tick_count = 0;
    float last_dt = 0.0f;
    uint64_t last_frame = 0;
};

class TestGameMode final : public IGameMode {
public:
    explicit TestGameMode(std::string_view p_id)
        : m_id(p_id), counters(std::make_shared<Counters>()) {
    }

    std::string_view GetId() const override { return m_id; }

    void OnEnter(GameSession&) override { ++counters->enter_count; }
    void OnExit(GameSession&) override { ++counters->exit_count; }

    void Tick(GameSession&, const GameFrameTime& p_time) override {
        ++counters->tick_count;
        counters->last_dt = p_time.dt;
        counters->last_frame = p_time.frame_index;
    }

    std::shared_ptr<Counters> counters;

private:
    std::string_view m_id;
};

static void DeleteTestGameMode(IGameMode* p_mode) {
    if (p_mode) {
        delete p_mode;
    }
}

static bool RegisterTestGame(GameModeFactory& factory) {
    return factory.Register(
        "test",
        []() -> IGameMode* { return new TestGameMode("test"); },
        DeleteTestGameMode);
}

TEST(GameModeFactory, register_and_create) {
    GameModeFactory factory;

    EXPECT_TRUE(RegisterTestGame(factory));

    // Registering the same id should fail.
    EXPECT_FALSE(RegisterTestGame(factory));

    auto mode = factory.Create("test");
    ASSERT_NE(mode, nullptr);
    EXPECT_EQ(mode->GetId(), "test");

    // Non-existent id returns nullptr.
    EXPECT_EQ(factory.Create("missing"), nullptr);
}

TEST(GameSession, start_tick_stop_calls_lifecycle) {
    GameModeFactory factory;

    EXPECT_TRUE(RegisterTestGame(factory));

    GameSession session(factory);

    EXPECT_TRUE(session.Start("test"));
    auto mode = dynamic_cast<const TestGameMode*>(session.GetMode());
    auto c = mode->counters;

    EXPECT_EQ(c->enter_count, 1);
    EXPECT_EQ(c->exit_count, 0);
    EXPECT_EQ(c->tick_count, 0);

    GameFrameTime ft{};
    ft.dt = 0.016f;
    ft.frame_index = 42;

    session.Tick(ft);

    EXPECT_EQ(c->tick_count, 1);
    EXPECT_FLOAT_EQ(c->last_dt, 0.016f);
    EXPECT_EQ(c->last_frame, 42);

    session.Stop();

    EXPECT_EQ(c->exit_count, 1);
}

TEST(GameSession, request_switch_then_commit_swaps_at_sync_point) {
    GameModeFactory factory;

    factory.Register(
        "A",
        []() -> IGameMode* { return new TestGameMode("A"); },
        DeleteTestGameMode);
    factory.Register(
        "B",
        []() -> IGameMode* { return new TestGameMode("B"); },
        DeleteTestGameMode);

    GameSession session(factory);

    ASSERT_TRUE(session.Start("A"));
    auto mode = dynamic_cast<const TestGameMode*>(session.GetMode());
    const auto a = mode->counters;
    ASSERT_NE(a, nullptr);

    EXPECT_EQ(a->enter_count, 1);
    EXPECT_EQ(a->exit_count, 0);

    // Request does NOT switch immediately.
    ASSERT_TRUE(session.RequestSwitch("B"));
    // The active mode is still A
    EXPECT_EQ(session.GetActiveModeId(), "A");

    // Still ticking A until commit.
    GameFrameTime ft{};
    ft.dt = 0.01f;
    ft.frame_index = 1;
    session.Tick(ft);
    EXPECT_EQ(a->tick_count, 1);

    // Commit at sync point.
    EXPECT_TRUE(session.CommitModeSwitch());
    EXPECT_EQ(session.GetActiveModeId(), "B");

    mode = dynamic_cast<const TestGameMode*>(session.GetMode());
    const auto b = mode->counters;
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(a->exit_count, 1);
    EXPECT_EQ(b->enter_count, 1);

    // Now ticks go to B.
    ft.frame_index = 2;
    session.Tick(ft);
    EXPECT_EQ(b->tick_count, 1);

    // No pending switch => false.
    EXPECT_FALSE(session.CommitModeSwitch());
}

TEST(GameSession, request_switch_fails_if_mode_missing) {
    GameModeFactory factory;

    auto a = std::make_shared<Counters>();
    EXPECT_TRUE(RegisterTestGame(factory));

    GameSession session(factory);
    ASSERT_TRUE(session.Start("test"));

    EXPECT_FALSE(session.RequestSwitch("Missing"));
    EXPECT_EQ(session.GetActiveModeId(), "test");
}

}  // namespace cave::gameplay
