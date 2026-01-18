#pragma once
#include "viewer_tab_id.h"

#include "engine/assets/guid.h"
#include "engine/ecs/entity.h"
#include "engine/runtime/scene_view.h"

#include "editor/enums.h"
#include "editor/undo_redo/undo_stack.h"
#include "editor/widgets/tool_bar.h"

namespace cave {

class CameraComponent;
class Document;
class TabId;
class Viewer;

struct CameraInputState;
struct ToolBarButtonDesc;

class ViewerTab : public ISceneViewProvider {
public:
    ViewerTab(EditorLayer& p_editor, Viewer& p_viewer);

    virtual ~ViewerTab() = default;

    virtual bool HandleInput(const InputEvent* p_input_event) = 0;

    void OnCreate(const Guid& p_guid);
    virtual void OnDestroy() {}

    void OnActivate();
    void OnDeactivate();

    virtual void DrawMainView(const CameraComponent& p_camera);
    virtual void DrawAssetInspector();

    virtual Document& GetDocument() const = 0;

    virtual Scene* GetScene() { return nullptr; }

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

    const char* GetDebugName() const override {
        return "ViewerTab";
    }

protected:
    virtual void OnCreateInternal(const Guid& p_guid) = 0;
    virtual void OnActivateInternal() {}
    virtual void OnDeactivateInternal() {}

    virtual void CreateDefaultCameraAndController();

    void CameraInputState2D(float p_timestep,
                            const ViewportInput& p_input,
                            CameraInputState& p_out_state);

    void CameraInputState3D(float p_timestep,
                            const ViewportInput& p_input,
                            CameraInputState& p_out_state);

    // @TODO: get rid of this
    static void CreateDefaultCamera2D(CameraComponent& p_out);

    const TabId m_id;
    EditorLayer& m_editor;
    Viewer& m_viewer;

    ecs::Entity m_selected;
    bool m_active{ false };

    // @TODO: camera controller
    std::shared_ptr<CameraComponent> m_camera;

private:
    std::string m_title;
};

}  // namespace cave