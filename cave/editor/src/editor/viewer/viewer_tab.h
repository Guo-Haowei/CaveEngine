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
struct ToolBarButtonDesc;
class Viewer;

class ViewerTab: public ISceneViewProvider {
public:
    ViewerTab(EditorLayer& p_editor, Viewer& p_viewer);

    virtual ~ViewerTab() = default;

    virtual bool HandleInput(const InputEvent* p_input_event) = 0;

    virtual void OnCreate(const Guid&);
    virtual void OnDestroy() {}

    virtual void OnActivate() {}
    virtual void OnDeactivate() {}

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

    void Update(float, bool) override {}

    void BuildViews(std::vector<SceneView>&, bool) override {}

    const char* GetDebugName() const override {
        return "ViewerTab";
    }

protected:
    virtual const CameraComponent& GetActiveCameraInternal() const = 0;

    static void CreateDefaultCamera2D(CameraComponent& p_out);
    static void CreateDefaultCamera3D(CameraComponent& p_out);

    const TabId m_id;
    EditorLayer& m_editor;
    Viewer& m_viewer;

    ecs::Entity m_selected;

private:
    std::string m_title;
};

}  // namespace cave