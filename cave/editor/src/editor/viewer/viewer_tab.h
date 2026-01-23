#pragma once
#include "viewer_tab_id.h"

#include "engine/assets/guid.h"
#include "engine/ecs/entity.h"
#include "engine/input/raw_input_consumer_interface.h"
#include "engine/runtime/scene_view.h"
#include "engine/scene/camera_controller.h"

#include "editor/enums.h"
#include "editor/undo_redo/undo_stack.h"
#include "editor/widgets/tool_bar.h"

namespace cave {

class Document;
class TabId;
class Viewer;

struct ToolBarButtonDesc;

class ViewerTab : public ISceneViewProvider, public IRawInputConsumer {
public:
    enum Dimension {
        DIMENSION_2,
        DIMENSION_3,
    };

    ViewerTab(EditorState& p_editor, Viewer& p_viewer, Dimension p_dimension);
    virtual ~ViewerTab();

    void OnCreate(const Guid& p_guid);
    virtual void OnDestroy() {}

    void OnActivate();
    void OnDeactivate();

    // @TODO: get rid of these two functions
    virtual void DrawMainView(const CameraComponent& p_camera);
    virtual void DrawAssetInspector();

    virtual Document& GetDocument() const = 0;

    virtual Scene* GetScene() = 0;

    ecs::Entity GetSelectedEntity() const { return m_selected; }
    void SetSelectedEntity(ecs::Entity p_selected);

    ecs::Entity GetCopiedEntity() const { return m_copied; }
    void SetCopiedEntity(ecs::Entity p_copied);

    const TabId& GetId() const { return m_id; }

    const Guid& GetGuid() const;

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

    void BuildViewsImpl(Scene* p_scene,
                        ecs::Entity p_camera,
                        std::vector<SceneView>& p_out_views,
                        bool p_is_opengl);

    // @TODO: refactor field
    const TabId m_id;
    EditorState& m_editor;
    Viewer& m_viewer;

    bool m_active{ false };

    ecs::Entity m_selected;
    ecs::Entity m_copied;

    ecs::Entity m_camera;

private:
    CameraInputState CreateCameraInputState2D(const std::vector<InputEvent>& p_events);
    CameraInputState CreateCameraInputState3D(const std::vector<InputEvent>& p_events);

    const Dimension m_dimension;
    std::string m_title;

    std::shared_ptr<ICameraController> m_camera_controller;
    CameraInputState m_camera_state;
};

}  // namespace cave