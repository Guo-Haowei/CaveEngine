#pragma once
#include "viewer_tab.h"

#include "engine/input/input_event.h"
#include "engine/scene/scene.h"

namespace cave {

class Viewer;

class SceneEditor : public ViewerTab {
public:
    SceneEditor(EditorLayer& p_editor, Viewer& p_viewer);

    bool HandleInput(const InputEvent* p_input_event) override;

    void Update() override;

protected:
    const CameraComponent& GetActiveCameraInternal() const override;

    GizmoAction m_state{ GizmoAction::Translate };
    std::shared_ptr<CameraComponent> m_camera;
};

}  // namespace cave
