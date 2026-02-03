#pragma once
#include "cave/core/math/Box.h"
#include "cave/render/ViewDesc.h"

#include "editor/document/SceneDocument.h"
#include "editor/panels/Tab.h"
#include "editor/services/IPickConsumer.h"

// @TODO: refactor
#include "editor/Enums.h"
#include "editor/camera/CameraController.h"

namespace cave {

enum ViewDimension : uint8_t {
    DIMENSION_2,
    DIMENSION_3,
};

class KeyState;

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

    DebugId GetDebugId() final { return m_debug_id; }

protected:
    void SubmitView();

    void DrawUIImpl() override;

    void DrawMainView();
    void DrawGizmo();

    Scene* GetResolvedScene();
    // void OnCreateInternal(const Guid& p_guid) final;

    // void OnActivateInternal() final;

    // const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;

    const DebugId m_debug_id;
    GizmoAction m_gizmo_action{ GizmoAction::Translate };

    SceneId m_preview_scene;

    std::array<const char*, 2> m_button_displays;
    std::array<const char*, 2> m_button_tooltips;

    ToolBarButtonDesc m_play_button;

    ViewDimension m_dim;
    int m_button_index{ 0 };
    math::Box2 m_rect;

    // @TODO: move to input controller
    std::unique_ptr<ICameraController> m_camera_controller;
    CameraComponent m_camera;
    TransformComponent m_camera_transform;
    GpuTextureId m_texture;
};

}  // namespace cave
