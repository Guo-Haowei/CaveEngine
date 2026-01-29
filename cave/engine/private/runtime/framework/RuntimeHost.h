#pragma once
#include "cave/core/ids/SceneId.h"

#include "engine/private/runtime/scene/SceneScheduler.h"

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
    SceneId existing;

    static SceneSource FromPath(std::string p_path) {
        return { Type::FromPath, std::move(p_path), {} };
    }

    static SceneSource FromExisting(SceneId p_scene_id) {
        return { Type::FromExisting, "", p_scene_id };
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
    // IInputProvider* input_provider;

    RuntimeStartParams(SceneSource p_source)
        : source(std::move(p_source)) {}
};

class RuntimeHost final : public ISceneTickContributor {
public:
    RuntimeHost(IApplication& p_app);
    ~RuntimeHost();

    void Start(const RuntimeStartParams& p_params);
    void Stop();

    void Tick(const GameFrameTime& p_frame);

    void CollectSceneTicks(std::vector<SceneTickRequest>& p_out) override;

    GameSession* GetSession() const { return m_session.get(); }

    SceneId GetSceneId() const { return m_scene_id; }

    DebugId GetDebugId() override { return m_debug_id; }

private:
    IApplication& m_app;
    const DebugId m_debug_id;

    std::unique_ptr<GameSession> m_session;
    SceneId m_scene_id;
};

}  // namespace cave
