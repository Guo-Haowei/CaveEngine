#pragma once
#include "viewer_tab.h"

#include "engine/input/input_event.h"
#include "engine/scene/scene.h"

namespace cave {

class Viewer;

class SceneEditor : public ViewerTab {
public:
    SceneEditor(EditorLayer& p_editor, Viewer& p_viewer)
        : ViewerTab(p_editor, p_viewer) {
        m_title = "Scene Editor";
    }

    bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event) override;

    void Update() override;

protected:
    GizmoAction m_state{ GizmoAction::Translate };
};

}  // namespace cave
