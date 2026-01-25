#pragma once
#include "engine/scene/camera_component.h"

#include "editor/viewer/ViewerTab.h"

namespace cave {

class SceneDocument;

class SceneEditor : public ViewerTab {
public:
    SceneEditor(EditorState& p_editor, Viewer& p_viewer, ViewerTab::Dimension p_dimension);

    void OnDestroy() final;

    void DrawMainView(const CameraComponent& p_camera) final;

    Document& GetDocument() const final;

    Scene* GetScene() final;

    void BuildViews(std::vector<SceneView>& p_out_views,
                    bool p_is_opengl) final;

protected:
    void OnCreateInternal(const Guid& p_guid) final;

    void OnActivateInternal() final;

    const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;

    GizmoAction m_state{ GizmoAction::Translate };

    void Select(const Vector2f& p_cursor);

    std::shared_ptr<SceneDocument> m_document;

    ToolBarButtonDesc m_play_button;
    ToolBarButtonDesc m_pause_button;
};

}  // namespace cave
