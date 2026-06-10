#pragma once
#include "engine/private/runtime/framework/AppState.h"
#include "engine/private/runtime/projects/ProjectInfo.h"

namespace cave {

class ProjectManager;

class ProjectBrowserState final : public AppState {
public:
    ProjectBrowserState(IApplication& app);

    void onEnter(const StateRequest& args) override;

    void onExit() override;

    void tick(const FrameTime& time) override;

    Option<StateRequest> popRequest() override;

#if USING(DEBUG_BUILD)
    DebugId debugId() const { return debug_id_; }
#endif

private:
    void drawUI();
    void drawRecentProjects();
    void drawSideBar();

    ProjectManager& project_manager_;
    std::vector<ProjectInfo> project_list_{};

    bool request_fired_{ false };
    Option<StateRequest> request_{};

    const DebugId debug_id_;
};

}  // namespace cave
