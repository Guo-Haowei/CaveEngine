#include "engine/runtime/gameplay/GameSession.h"

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
    explicit TestGameMode(std::string_view p_id, std::shared_ptr<Counters> p_counters)
        : m_id(p_id), m_counters(p_counters) {
    }

    std::string_view GetId() const override { return m_id; }

    void OnEnter(GameSession&) override { ++m_counters->enter_count; }
    void OnExit(GameSession&) override { ++m_counters->exit_count; }

    void Tick(GameSession&, const GameFrameTime& p_time) override {
        ++m_counters->tick_count;
        m_counters->last_dt = p_time.dt;
        m_counters->last_frame = p_time.frame_index;
    }

private:
    std::string_view m_id;
    std::shared_ptr<Counters> m_counters;
};

TEST(GameModeFactory, register_and_create) {
    GameModeFactory factory;

    auto c = std::make_shared<Counters>();
    const bool ok = factory.Register("test", [&c] {
        return std::make_unique<TestGameMode>("test", c);
    });

    EXPECT_TRUE(ok);

    // Registering the same id should fail.
    EXPECT_FALSE(factory.Register("test", [&c] {
        return std::make_unique<TestGameMode>("test", c);
    }));

    auto mode = factory.Create("test");
    ASSERT_NE(mode, nullptr);
    EXPECT_EQ(mode->GetId(), "test");

    // Non-existent id returns nullptr.
    EXPECT_EQ(factory.Create("missing"), nullptr);
}

TEST(GameSession, start_tick_stop_calls_lifecycle) {
    GameModeFactory factory;

    auto c = std::make_shared<Counters>();

    factory.Register("test", [&c] {
        return std::make_unique<TestGameMode>("test", c);
    });

    GameSession session(factory);

    EXPECT_TRUE(session.Start("test"));

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

    auto a = std::make_shared<Counters>();
    auto b = std::make_shared<Counters>();
    factory.Register("A", [&a] {
        auto ptr = std::make_unique<TestGameMode>("A", a);
        return ptr;
    });
    factory.Register("B", [&b] {
        auto ptr = std::make_unique<TestGameMode>("B", b);
        return ptr;
    });

    GameSession session(factory);

    ASSERT_TRUE(session.Start("A"));
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
    factory.Register("A", [&a] {
        return std::make_unique<TestGameMode>("A", a);
    });

    GameSession session(factory);
    ASSERT_TRUE(session.Start("A"));

    EXPECT_FALSE(session.RequestSwitch("Missing"));
    EXPECT_EQ(session.GetActiveModeId(), "A");
}

}  // namespace cave::gameplay
