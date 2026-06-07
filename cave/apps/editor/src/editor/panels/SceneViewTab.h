#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/core/math/Rect.h"
#include "cave/runtime/view/ViewDesc.h"

#include "editor/document/SceneDocument.h"
#include "editor/panels/Tab.h"
#include "editor/services/IPickConsumer.h"

// @TODO: refactor
#include "editor/Enums.h"
#include "editor/camera/CameraController.h"

namespace cave {

class KeyState;
class ViewManager;

enum ViewDimension : uint8_t {
    DIMENSION_2,
    DIMENSION_3,
};

class SceneViewTab : public Tab,
                     public IPickConsumer,
                     public ISceneTickContributor {
public:
    SceneViewTab(EditorState& p_editor,
                 DocId p_doc_id,
                 SceneId p_preview_scene_id,
                 ViewDimension p_dim);

    void OnCreate() override;
    void OnDestroy() override;

    void CollectSceneTicks(std::vector<SceneTickRequest>& p_out) override;

    Option<PickData> GetPickData(const math::Vector2f& p_pos_screen) override;

    void OnInputEvents(const InputFrame& p_input) override;

    ViewId GetViewId() const override { return m_view_id; }

    DebugId debugId() const final { return m_debug_id; }

protected:
    void SubmitView();

    void DrawUIImpl() override;

    void UpdateRect(math::FloatRect& p_out_rect);
    void DrawMainView(const math::FloatRect& p_rect);
    void DrawGizmo(const math::FloatRect& p_rect);

    Scene* GetResolvedScene();
    // void OnCreateInternal(const Guid& p_guid) final;

    // void OnActivateInternal() final;

    // const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;

    const DebugId m_debug_id;
    ViewManager& m_view_manager;
    GizmoAction m_gizmo_action{ GizmoAction::Translate };

    SceneId m_preview_scene;

    std::array<const char*, 2> m_button_displays;
    std::array<const char*, 2> m_button_tooltips;

    ToolBarButtonDesc m_play_button;

    ViewDimension m_dim;
    int m_button_index{ 0 };

    // @TODO: move to input controller
    std::unique_ptr<ICameraController> m_camera_controller;
    CameraComponent m_camera;
    TransformComponent m_camera_transform;
    GpuTextureId m_texture;
    ViewId m_view_id;
};

}  // namespace cave
