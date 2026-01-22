#pragma once
#include "engine/runtime/app_state.h"

namespace cave {

class ProjectBrowserState : public AppState {
    struct ProjectItem {
        std::string name;
        std::string path;
    };

public:
    ProjectBrowserState(Application& p_app);

    void OnEnter(const StateRequest& p_args) final;

    void OnExit() final;

    void Tick(float p_timestep) final;

    Option<StateRequest> PopRequest() final;

#if USING(DEBUG_BUILD)
    const char* GetDebugName() final { return "ProjectBrowser"; }
#endif

private:
    void DrawUI();
    void DrawRecentProjects();

    std::vector<ProjectItem> m_projects{};
    bool m_request_fired{ false };
    bool m_modal_popped{ false };
    Option<StateRequest> m_request{};
};

}  // namespace cave
