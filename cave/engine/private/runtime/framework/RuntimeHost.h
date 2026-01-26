#pragma once

namespace cave {

class GameSession;
class IApplication;
class Scene;
struct GameFrameTime;

struct SceneSource {
    enum class Type : uint8_t {
        FromPath,
        FromExisting,
    } type;
    std::string path;
    Scene* existing{ nullptr };

    static SceneSource FromPath(std::string p_path) {
        return { Type::FromPath, std::move(p_path), nullptr };
    }

    static SceneSource FromExisting(Scene* p_world) {
        return { Type::FromExisting, "", p_world };
    }
};

struct RuntimeStartParams {
    SceneSource source;
    enum class Mode : uint8_t {
        Standalone,  // Game runtime
        PIE,         // Play in Editor
    } mode;
    bool enable_rendering = true;
    bool enable_pause = false;

    std::string game_mode_id;

    // Input routing
    //IInputProvider* input_provider;

    RuntimeStartParams(SceneSource p_source)
        : source(std::move(p_source)) {}
};

class RuntimeHost {
public:
    RuntimeHost(IApplication& p_app);
    ~RuntimeHost();

    void Start(const RuntimeStartParams& p_params);
    void Stop();

    void Tick(const GameFrameTime& p_frame);

    GameSession* GetSession() const;
    Scene* GetScene() const;

private:
    IApplication& m_app;

    std::unique_ptr<GameSession> m_session;
    std::unique_ptr<Scene> m_world;
};

}  // namespace cave
