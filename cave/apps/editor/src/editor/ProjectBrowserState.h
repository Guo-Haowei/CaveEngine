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
    DebugId debugId() const { return m_debug_id; }
#endif

private:
    void selectProject(const ProjectInfo& project);
    void selectProject(std::string_view path);

    void drawUI();
    void drawRecentProjects();
    void drawSideBar();

    ProjectManager& m_project_manager;
    const DebugId m_debug_id;

    std::vector<ProjectInfo> m_project_list{};

    bool m_request_fired{ false };
    Option<StateRequest> m_request{};
};

}  // namespace cave
