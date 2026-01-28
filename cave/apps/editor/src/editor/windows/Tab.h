#pragma once
#include "editor/windows/EditorWindow.h"

#include "cave/runtime/input/IInputConsumer.h"
#include "engine/private/runtime/framework/SceneView.h"
#include "engine/private/runtime/scene/SceneScheduler.h"

#include "editor/document/DocumentTypes.h"

// @TODO: remove
#include "editor/widgets/ToolBar.h"
#include "engine/private/runtime/scene/CameraController.h"

namespace cave {

class KeyState;

enum ViewDimension : uint8_t{
    DIMENSION_2,
    DIMENSION_3,
};

class Tab : public EditorWindow,
            public ISceneViewProvider,
            public IInputConsumer,
            public ISceneTickContributor {
public:
    Tab(EditorState& p_editor,
        DocId p_doc_id,
        SceneId p_scene_id, // @TODO: decouple, tab doesn't always have a scene
        ViewDimension p_dim);

    const char* GetWindowId() const override { return m_window_id.c_str(); }

    void SetTitleAndId(std::string_view p_title, uint32_t p_idx);

    virtual void OnCreate();
    virtual void OnDestroy();

    // @TODO: let view service decides
    void BuildViews(std::vector<SceneView>&, bool) {}

    int GetPriority() const override { return 10; }

    // @TODO: workspace decide which view the input should route to
    void OnEvents(const std::vector<InputEvent>&) override {}

    ViewDimension GetDimension() const { return m_dim; }

    DocId GetDocId() const { return m_doc_id; }

    void CollectSceneTicks(std::vector<SceneTickRequest>&) override {}

protected:
    void UpdateInternal(float p_dt) override;

    //virtual const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const;

    const ViewDimension m_dim;
    DocId m_doc_id;
    std::string m_window_id;
    std::string m_title;
    uint32_t m_idx{ 0 };

    // @TODO: cleanup
    void BuildViewsImpl(SceneId p_scene_id,
                        ecs::Entity p_camera,
                        std::vector<SceneView>& p_out_views,
                        bool p_is_opengl);

    void SetupDefault2DCamera();
    void SetupDefault3DCamera();

    // @TODO: move to input controller
    ecs::Entity m_camera;
    CameraInputState CreateCameraInputState2D(const std::vector<InputEvent>& p_events, const KeyState& p_st);
    CameraInputState CreateCameraInputState3D(const std::vector<InputEvent>& p_events, const KeyState& p_st);
    std::shared_ptr<ICameraController> m_camera_controller;
    CameraInputState m_camera_state;
    SceneId m_scene_id; // @TODO: tab doesn't always own a scene

    Scene* GetResolvedScene();
};

}  // namespace cave
