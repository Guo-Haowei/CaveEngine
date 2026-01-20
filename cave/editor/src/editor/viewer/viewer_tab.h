#pragma once
#include "viewer_tab_id.h"

#include "engine/assets/guid.h"
#include "engine/ecs/entity.h"
#include "engine/runtime/scene_view.h"

#include "editor/enums.h"
#include "editor/undo_redo/undo_stack.h"
#include "editor/widgets/tool_bar.h"

namespace cave {

class Document;
class ICameraController;
class TabId;
class Viewer;

struct CameraInputState;

struct ToolBarButtonDesc;

class ViewerTab : public ISceneViewProvider {
public:
    enum Dimension {
        DIMENSION_2,
        DIMENSION_3,
    };

    ViewerTab(EditorState& p_editor, Viewer& p_viewer, Dimension p_dimension);

    virtual ~ViewerTab() = default;

    virtual bool HandleInput(const InputEvent* p_input_event) = 0;

    void OnCreate(const Guid& p_guid);
    virtual void OnDestroy() {}

    void OnActivate();
    void OnDeactivate();

    // @TODO: get rid of these two functions
    virtual void DrawMainView(const CameraComponent& p_camera);
    virtual void DrawAssetInspector();

    virtual Document& GetDocument() const = 0;

    virtual Scene* GetScene() = 0;

    void SelectEntity(ecs::Entity p_selected);
    ecs::Entity GetSelectedEntity() const { return m_selected; }

    const TabId& GetId() const { return m_id; }

    const Guid& GetGuid() const;

    const std::string& GetTitle() const {
        return m_title;
    }

    virtual const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const = 0;

    void Update(float p_timestep,
                const ViewportInput& p_input,
                bool p_focused) override;

    void BuildViews(std::vector<SceneView>& p_out_views,
                    bool p_is_opengl) override;

    Dimension GetDimension() const { return m_dimension; }

protected:
    virtual void OnCreateInternal(const Guid& p_guid) = 0;
    virtual void OnActivateInternal() {}
    virtual void OnDeactivateInternal() {}

    void SetupDefault2DCamera();
    void SetupDefault3DCamera();

    void CameraInputState2D(float p_timestep,
                            const ViewportInput& p_input,
                            CameraInputState& p_out_state);

    void CameraInputState3D(float p_timestep,
                            const ViewportInput& p_input,
                            CameraInputState& p_out_state);

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

    ecs::Entity m_camera;
    std::shared_ptr<ICameraController> m_camera_controller;

private:
    const Dimension m_dimension;
    std::string m_title;
};

}  // namespace cave