#pragma once
#include "editor/document/DocumentTypes.h"

// @TODO: refactor

#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/input/IInputConsumer.h"
#include "cave/runtime/scene/SceneId.h"

#include "engine/private/assets/guid.h"
#include "engine/private/runtime/framework/SceneView.h"
#include "engine/private/runtime/scene/CameraController.h"
#include "engine/private/runtime/scene/SceneScheduler.h"

#include "editor/Enums.h"
#include "editor/undo_redo/UndoStack.h"
#include "editor/widgets/ToolBar.h"

namespace cave {

#if 0
class ISceneRegistry;
class KeyState;
class ViewerTabId;
class Viewer;

struct ToolBarButtonDesc;

class ViewerTab : public ISceneViewProvider,
                  public IInputConsumer,
                  public ISceneTickContributor {
public:
    enum Dimension {
        DIMENSION_2,
        DIMENSION_3,
    };

    ViewerTab(EditorState& p_editor,
              DocId p_doc_id,
              Viewer& p_viewer,
              Dimension p_dimension);
    virtual ~ViewerTab();

    void OnCreate(const Guid& p_guid);
    virtual void OnDestroy() {}

    void OnActivate();
    void OnDeactivate();

    void CollectSceneTicks(std::vector<SceneTickRequest>& p_out) override;

    DocId GetDocId() const { return m_doc_id; }

    // @TODO: fix this
    virtual SceneId GetSceneId() const {
        return SceneId{};
    }

    // @TODO: get rid of these two functions
    virtual void DrawMainView(const CameraComponent& p_camera);
    virtual void DrawAssetInspector();

    const ViewerTabId& GetId() const { return m_id; }

    const std::string& GetTitle() const {
        return m_title;
    }

    void Update(float p_timestep);

    virtual const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const = 0;

    void BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) override;

    int GetPriority() const override { return 10; }

    void OnEvents(const std::vector<InputEvent>& p_events) override;

    Dimension GetDimension() const { return m_dimension; }

protected:
    virtual void OnCreateInternal(const Guid& p_guid) = 0;
    virtual void OnActivateInternal() {}
    virtual void OnDeactivateInternal() {}

    void SetupDefault2DCamera();
    void SetupDefault3DCamera();

    void BuildViewsImpl(SceneId p_scene_id,
                        ecs::Entity p_camera,
                        std::vector<SceneView>& p_out_views,
                        bool p_is_opengl);

    // @TODO: deprecate
    Scene* GetResolvedScene();

    // @TODO: refactor field
    const ViewerTabId m_id;
    EditorState& m_editor;
    Viewer& m_viewer;
    ISceneRegistry& m_scene_manager;

    const DocId m_doc_id;

    bool m_active{ false };

    ecs::Entity m_camera;

private:
    CameraInputState CreateCameraInputState2D(const std::vector<InputEvent>& p_events, const KeyState& p_st);
    CameraInputState CreateCameraInputState3D(const std::vector<InputEvent>& p_events, const KeyState& p_st);

    const Dimension m_dimension;
    std::string m_title;

    std::shared_ptr<ICameraController> m_camera_controller;
    CameraInputState m_camera_state;
};
#endif

}  // namespace cave