#pragma once
#include "viewer_tab_id.h"

#include "engine/assets/guid.h"
#include "editor/enums.h"
#include "editor/undo_redo/undo_stack.h"

namespace cave {

class CameraComponent;
class Document;
class TabId;

class ViewerTab {
public:
    ViewerTab(EditorLayer& p_editor, Viewer& p_viewer);

    virtual ~ViewerTab() = default;

    virtual bool HandleInput(const InputEvent* p_input_event);

    virtual void OnCreate(const Guid&);
    virtual void OnDestroy() {}

    virtual void OnActivate() {}
    virtual void OnDeactivate() {}

    virtual void DrawMainView();
    virtual void DrawAssetInspector();

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
    virtual const CameraComponent& GetActiveCameraInternal() const = 0;

    static std::shared_ptr<CameraComponent> CreateDefaultCamera2D();
    static std::shared_ptr<CameraComponent> CreateDefaultCamera3D();

    const TabId m_id;
    EditorLayer& m_editor;
    Viewer& m_viewer;

private:
    std::string m_title;
};

}  // namespace cave