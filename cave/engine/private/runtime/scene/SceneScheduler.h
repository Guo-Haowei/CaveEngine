#pragma once
#include "cave/runtime/scene/SceneId.h"

namespace cave {

class SceneManager;
class IScriptManager;

enum class SceneTickMode {
    Editor,
    Simulation,
};

struct SceneTickRequest {
    SceneTickMode mode;
    SceneId scene_id;
};

class SceneScheduler {
public:
    SceneScheduler(SceneManager& p_scene_manager,
                   IScriptManager& p_script_manager)
        : m_scene_manager(p_scene_manager)
        , m_script_manager(p_script_manager) {
    }

    void Add(const SceneTickRequest& p_request);

    void Tick(float p_dt);

private:
    IScriptManager& m_script_manager;
    SceneManager& m_scene_manager;
    std::vector<SceneTickRequest> m_requests;
};

}  // namespace cave
