#pragma once
#include "viewer_tab_id.h"

#include "engine/assets/guid.h"
#include "editor/enums.h"
#include "editor/undo_redo/undo_stack.h"
#include "editor/widgets/widget.h"

namespace cave {

class CameraComponent;
class Document;
class TabId;

class ViewerTab {
public:
    ViewerTab(EditorLayer& p_editor, Viewer& p_viewer);

    virtual ~ViewerTab() = default;

    virtual bool HandleInput(const InputEvent* p_input_event) = 0;

    virtual void OnCreate(const Guid&);
    virtual void OnDestroy() {}

    virtual void OnActivate() {}
    virtual void OnDeactivate() {}

    void DrawToolBar();
    virtual void DrawMainView(const CameraComponent& p_camera);
    virtual void DrawAssetInspector() = 0;

    virtual Document& GetDocument() const = 0;

    const TabId& GetId() const { return m_id; }

    const Guid& GetGuid() const;

    const std::string& GetTitle() const {
        return m_title;
    }

    const CameraComponent& GetActiveCamera() const {
        return GetActiveCameraInternal();
    }

    CameraComponent& GetActiveCamera() {
        return const_cast<CameraComponent&>(GetActiveCameraInternal());
    }

protected:
    virtual const std::vector<ToolBarButtonDesc>& GetToolBarButtons() const = 0;

    virtual const CameraComponent& GetActiveCameraInternal() const = 0;

    static void CreateDefaultCamera2D(CameraComponent& p_out);
    static void CreateDefaultCamera3D(CameraComponent& p_out);

    const TabId m_id;
    EditorLayer& m_editor;
    Viewer& m_viewer;

private:
    std::string m_title;
};

}  // namespace cave