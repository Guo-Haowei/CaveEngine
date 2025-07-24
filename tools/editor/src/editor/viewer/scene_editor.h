#pragma once
#include "viewer_tab.h"

#include "engine/input/input_event.h"
#include "engine/scene/camera_component.h"

namespace cave {

class SceneDocument;
class Viewer;

class SceneEditor : public ViewerTab {
public:
    SceneEditor(EditorLayer& p_editor, Viewer& p_viewer);

    bool HandleInput(const InputEvent* p_input_event) override;

    void OnCreate(const Guid& p_guid) override;

    void OnDestroy() override;

    void OnActivate() override;

    void DrawMainView(const CameraComponent& p_camera) override;

    void DrawAssetInspector() override;

    Document& GetDocument() const override;

    Scene* GetScene() override;

protected:
    const CameraComponent& GetActiveCameraInternal() const override;

    const std::vector<ToolBarButtonDesc>& GetToolBarButtons() const override;

    GizmoAction m_state{ GizmoAction::Translate };

    std::shared_ptr<SceneDocument> m_document;

    std::array<CameraComponent, 2> m_cameras;
    mutable int m_camera_idx = 0;
};

}  // namespace cave
