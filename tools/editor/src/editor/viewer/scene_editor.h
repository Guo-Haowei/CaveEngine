#pragma once
#include "viewer_tab.h"

#include "engine/input/input_event.h"
#include "engine/scene/scene.h"

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

    void DrawMainView() override;

    void DrawAssetInspector() override;

    Document& GetDocument() const override;

protected:
    const CameraComponent& GetActiveCameraInternal() const override;

    GizmoAction m_state{ GizmoAction::Translate };

    std::shared_ptr<CameraComponent> m_camera;
    std::shared_ptr<SceneDocument> m_document;
};

}  // namespace cave
