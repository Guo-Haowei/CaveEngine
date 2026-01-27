#pragma once
#include "engine/private/runtime/scene/CameraComponent.h"

#include "editor/viewer/ViewerTab.h"

namespace cave {

class SceneDocument;

class SceneEditor : public ViewerTab {
public:
    SceneEditor(EditorState& p_editor, Viewer& p_viewer, ViewerTab::Dimension p_dimension);

    void OnDestroy() final;

    void DrawMainView(const CameraComponent& p_camera) final;

    OldDocument& GetDocument() const final;

    SceneId GetSceneId() const final;

    void BuildViews(std::vector<SceneView>& p_out_views,
                    bool p_is_opengl) final;

protected:
    void OnCreateInternal(const Guid& p_guid) final;

    void OnActivateInternal() final;

    const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;

    GizmoAction m_state{ GizmoAction::Translate };

    void Select(const Vector2f& p_cursor);

    std::array<const char*, 2> m_button_displays;
    std::array<const char*, 2> m_button_tooltips;

    int m_button_index{ 0 };
    std::shared_ptr<SceneDocument> m_document;
    ToolBarButtonDesc m_play_button;
};

}  // namespace cave
