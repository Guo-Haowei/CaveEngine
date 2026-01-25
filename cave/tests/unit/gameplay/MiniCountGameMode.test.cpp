#include "engine/private/runtime/gameplay/GameSession.h"

namespace cave::gameplay {

static constexpr uint64_t kDecision_Add = 1;

enum class MatchResult {
    Ongoing,
    Player0Win,
    Player1Win,
};

struct Config {
    int target = 9;
};

class MiniCountGameMode final : public IGameMode {
public:
    explicit MiniCountGameMode(Config cfg = {})
        : m_cfg(cfg) {}

    std::string_view GetId() const override { return "minicount"; }

    void OnEnter(GameSession& p_session) override {
        m_total = 0;
        m_current_player = 0;
        m_result = MatchResult::Ongoing;

        p_session.GetPlayer(0)->OnMatchStart(p_session, *this, 0);
        p_session.GetPlayer(1)->OnMatchStart(p_session, *this, 1);
    }

    void OnExit(GameSession& p_session) override {
        p_session.GetPlayer(0)->OnMatchEnd(p_session, *this, 0);
        p_session.GetPlayer(1)->OnMatchEnd(p_session, *this, 1);
    }

    void Tick(GameSession& p_session, const GameFrameTime& p_time) override {
        if (m_result != MatchResult::Ongoing) {
            return;
        }

        p_session.GetPlayer(0)->Tick(p_session, *this, 0, p_time);
        p_session.GetPlayer(1)->Tick(p_session, *this, 1, p_time);

        TryConsumeDecision(p_session, p_time);
    }

    int Total() const { return m_total; }
    int CurrentPlayer() const { return m_current_player; }
    MatchResult Result() const { return m_result; }

private:
    bool TryConsumeDecision(GameSession& p_session, const GameFrameTime&) {
        IPlayerAgent* agent = p_session.GetPlayer(m_current_player);
        if (!agent) {
            return false;
        }

        GameDecision decision;
        if (!agent->PollDecision(p_session, *this, m_current_player, decision)) {
            return false;
        }

        int add = 0;
        if (!DecodeAdd(decision, add)) {
            return false;
        }

        m_total += add;

        // printf("player %d played +%d, total is %d\n", m_current_player, add, m_total);

        // Win check
        if (m_total >= m_cfg.target) {
            m_result = (m_current_player == 0) ? MatchResult::Player0Win : MatchResult::Player1Win;
            return true;
        }

        // Next turn
        m_current_player ^= 1;
        return true;
    }

    static bool DecodeAdd(const GameDecision& p_decision, int& p_out_add) {
        if (p_decision.type != kDecision_Add) {
            return false;
        }
        if (p_decision.payload.size() != 1) {
            return false;
        }
        const uint8_t v = p_decision.payload[0];
        if (v != 1 && v != 2) {
            return false;
        }
        p_out_add = static_cast<int>(v);
        return true;
    }

private:
    Config m_cfg;

    int m_total = 0;
    int m_current_player = 0;
    MatchResult m_result = MatchResult::Ongoing;
};

class GreedyAddAgent final : public IPlayerAgent {
public:
    void OnMatchStart(GameSession&, const IGameMode&, int) override {}
    void OnMatchEnd(GameSession&, const IGameMode&, int) override {}

    bool PollDecision(cave::GameSession&,
                      const cave::IGameMode&,
                      int,
                      cave::GameDecision& p_out_decision) override {
        p_out_decision.type = kDecision_Add;
        p_out_decision.payload = { 2 };
        return true;
    }
};

class ScriptedAddAgent final : public IPlayerAgent {
public:
    explicit ScriptedAddAgent(std::deque<uint8_t> p_seq)
        : m_seq(std::move(p_seq)) {}

    void OnMatchStart(GameSession&, const IGameMode&, int) override {}
    void OnMatchEnd(GameSession&, const IGameMode&, int) override {}

    bool PollDecision(GameSession&,
                      const cave::IGameMode&,
                      int,
                      GameDecision& p_out_decision) override {

        if (m_seq.empty()) {
            return false;
        }

        const uint8_t add = m_seq.front();
        m_seq.pop_front();

        p_out_decision.type = kDecision_Add;
        p_out_decision.payload = { add };
        return true;
    }

private:
    std::deque<uint8_t> m_seq;
};

static inline void RegisterMiniCount(cave::GameModeFactory& factory) {
    factory.Register("minicount", [] {
        return std::make_unique<MiniCountGameMode>(Config{ .target = 9 });
    });
}

TEST(MiniCountGameMode, scripted_vs_greedy_player0_wins) {
    GameModeFactory factory;
    RegisterMiniCount(factory);

    GameSession session(factory);
    session.AddPlayer(std::make_unique<ScriptedAddAgent>(std::deque<uint8_t>{ 2, 2, 1 }));
    session.AddPlayer(std::make_unique<GreedyAddAgent>());

    ASSERT_TRUE(session.Start("minicount"));

    auto* mode = dynamic_cast<MiniCountGameMode*>(session.GetMode());
    ASSERT_NE(mode, nullptr);

    // Simulate frames until game ends (turn-based consumes 1 decision per frame)
    for (uint64_t f = 0; f < 32; ++f) {
        GameFrameTime ft{};
        ft.dt = 1.0f / 60.0f;
        ft.frame_index = f;

        session.Tick(ft);

        if (mode->Result() != MatchResult::Ongoing) {
            break;
        }
    }

    EXPECT_EQ(mode->Result(), MatchResult::Player0Win);
    EXPECT_GE(mode->Total(), 9);
}

TEST(MiniCountGameMode, greedy_vs_scripted_player1_wins) {
    GameModeFactory factory;
    RegisterMiniCount(factory);

    GameSession session(factory);
    session.AddPlayer(std::make_unique<GreedyAddAgent>());
    session.AddPlayer(std::make_unique<ScriptedAddAgent>(std::deque<uint8_t>{ 1, 1, 1 }));

    ASSERT_TRUE(session.Start("minicount"));

    auto* mode = dynamic_cast<MiniCountGameMode*>(session.GetMode());
    ASSERT_NE(mode, nullptr);

    // Simulate frames until game ends (turn-based consumes 1 decision per frame)
    for (uint64_t f = 0; f < 32; ++f) {
        GameFrameTime ft{};
        ft.dt = 1.0f / 60.0f;
        ft.frame_index = f;

        session.Tick(ft);

        if (mode->Result() != MatchResult::Ongoing) {
            break;
        }
    }

    EXPECT_EQ(mode->Result(), MatchResult::Player1Win);
    EXPECT_GE(mode->Total(), 9);
}

}  // namespace cave::gameplay
