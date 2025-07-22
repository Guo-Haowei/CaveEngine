#pragma once

namespace cave {

class Application;

enum class GameMode : uint8_t {
    Editor,
    Gameplay,
    CutScene,
    Loading,
    Paused,
};

class ModeManager {
public:
    ModeManager(GameMode p_mode, Application& p_app)
        : m_mode(p_mode)
        , m_app(p_app) {}

    GameMode GetMode() const { return m_mode; }

    virtual void SetMode(GameMode) {}

protected:
    GameMode m_mode;
    Application& m_app;
};

}  // namespace cave
