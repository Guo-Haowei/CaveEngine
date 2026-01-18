#pragma once
#include "engine/input/input_event.h"
#include "engine/scene/camera_component.h"

#include "editor/viewer/viewer_tab.h"

namespace cave {

class SceneDocument;

class SceneEditor : public ViewerTab {
public:
    SceneEditor(EditorLayer& p_editor, Viewer& p_viewer);

    bool HandleInput(const InputEvent* p_input_event) final;

    void OnCreate(const Guid& p_guid) final;

    void OnDestroy() final;

    void OnActivate() final;

    void DrawMainView(const CameraComponent& p_camera) final;

    Document& GetDocument() const final;

    Scene* GetScene() final;

    void Update(float p_timestep, bool p_focused) final;

    void BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) final;

    const char* GetDebugName() const final {
        return "SceneEditor";
    }

protected:
    const CameraComponent& GetActiveCameraInternal() const override;

    const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const override;

    GizmoAction m_state{ GizmoAction::Translate };

    void Select(const Vector2f& p_cursor);

    std::shared_ptr<SceneDocument> m_document;

    std::array<CameraComponent, 2> m_cameras;
    mutable int m_camera_idx = 0;

    ToolBarButtonDesc m_play_button;
    ToolBarButtonDesc m_pause_button;
    ToolBarButtonDesc m_toggle_view_button;
};

}  // namespace cave
