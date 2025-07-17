#pragma once
#include "viewer_tab.h"

#include "engine/input/input_event.h"
#include "engine/scene/scene.h"

namespace cave {

class Viewer;

class EditorTool : public ViewerTab {
public:
    EditorTool(EditorLayer& p_editor, Viewer* p_viewer)
        : ViewerTab(p_editor), m_viewer(p_viewer) {}

    bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event) override;

    void Update(Scene* p_scene) override;

    bool Is2D() const override { return false; }

    const char* GetName() const override { return "Editor"; }
    const std::string& GetTile() const override { return m_title; }

protected:
    GizmoAction m_state{ GizmoAction::Translate };

    Viewer* m_viewer;
    std::string m_title{ "Editor" };
};

}  // namespace cave
