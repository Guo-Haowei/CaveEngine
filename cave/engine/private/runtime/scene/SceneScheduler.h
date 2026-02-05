#pragma once
#include "cave/core/ids/DebugId.h"
#include "cave/core/ids/SceneId.h"

namespace cave {

struct FrameTime;
class ISceneRegistry;
class IScriptManager;

enum class SceneTickMode {
    Editor,
    Simulation,
};

struct SceneTickRequest {
    SceneTickMode mode;
    SceneId scene_id;
};

class ISceneTickContributor {
public:
    virtual ~ISceneTickContributor() = default;

    virtual void CollectSceneTicks(std::vector<SceneTickRequest>& p_out) = 0;

    virtual DebugId GetDebugId() = 0;
};

class SceneScheduler {
public:
    SceneScheduler(ISceneRegistry& p_scene_manager,
                   IScriptManager& p_script_manager)
        : m_scene_manager(p_scene_manager)
        , m_script_manager(p_script_manager) {
    }

    bool Register(ISceneTickContributor* p_contributor);
    bool Unregister(ISceneTickContributor* p_contributor);

    void Tick(const FrameTime& p_time);

private:
    IScriptManager& m_script_manager;
    ISceneRegistry& m_scene_manager;

    std::vector<ISceneTickContributor*> m_contributors;
};

}  // namespace cave
