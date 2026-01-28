#pragma once
#include "editor/document/SceneDocument.h"
#include "editor/panels/Tab.h"

// @TODO: refactor
#include "editor/Enums.h"
#include "engine/private/runtime/scene/CameraController.h"

namespace cave {

enum ViewDimension : uint8_t {
    DIMENSION_2,
    DIMENSION_3,
};

class KeyState;

class SceneViewTab : public Tab,
                     public ISceneTickContributor {
public:
    SceneViewTab(EditorState& p_editor,
                 DocId p_doc_id,
                 SceneId p_preview_scene_id,
                 ViewDimension p_dim);

    void OnCreate() override;
    void OnDestroy() override;

    void CollectSceneTicks(std::vector<SceneTickRequest>& p_out) override;

    void OnInputEvents(const std::vector<InputEvent>& p_events) override;

    void Tick(float p_dt) override;

    // @TODO: cleanup
    void BuildViews(std::vector<SceneView>& p_out_views,
                    bool p_is_opengl) {
        BuildViewsImpl(m_preview_scene, m_camera, p_out_views, p_is_opengl);
    }

protected:
    void BuildViewsImpl(SceneId p_scene_id,
                        ecs::Entity p_camera,
                        std::vector<SceneView>& p_out_views,
                        bool p_is_opengl);

    // void OnCreateInternal(const Guid& p_guid) final;

    // void OnActivateInternal() final;

    // const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;

    GizmoAction m_state{ GizmoAction::Translate };

    void Select(const Vector2f& p_cursor);

    SceneId m_preview_scene;

    std::array<const char*, 2> m_button_displays;
    std::array<const char*, 2> m_button_tooltips;

    ToolBarButtonDesc m_play_button;

    int m_button_index{ 0 };
    ViewDimension m_dim;

    // @TODO: move to input controller
    void SetupDefault2DCamera();
    void SetupDefault3DCamera();
    std::shared_ptr<ICameraController> m_camera_controller;
    CameraInputState m_camera_state;
    ecs::Entity m_camera;
    Scene* GetResolvedScene();
    CameraInputState CreateCameraInputState2D(const std::vector<InputEvent>& p_events, const KeyState& p_st);
    CameraInputState CreateCameraInputState3D(const std::vector<InputEvent>& p_events, const KeyState& p_st);
};

}  // namespace cave
