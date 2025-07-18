#pragma once
#include "viewer_tab_id.h"

#include "engine/assets/guid.h"
#include "editor/enums.h"
#include "editor/undo_redo/undo_stack.h"

namespace cave {

class CameraComponent;
class TabId;

class ViewerTab {
public:
    ViewerTab(EditorLayer& p_editor, Viewer& p_viewer);

    virtual ~ViewerTab() = default;

    virtual bool HandleInput(const InputEvent* p_input_event);

    virtual void OnCreate(const Guid&) {}
    virtual void OnDestroy() {}

    virtual void OnActivate() {}
    virtual void OnDeactivate() {}

    virtual bool IsDirty() { return false; }

    virtual void Draw();

    virtual void Update() {}

    UndoStack& GetUndoStack() { return m_undo_stack; }

    const TabId& GetId() const { return m_id; }

    const std::string& GetTitle() const {
        return m_title;
    }

    const Guid& GetGuid() const { return m_guid; }

    const CameraComponent& GetActiveCamera() const {
        return GetActiveCameraInternal();
    }

    CameraComponent& GetActiveCamera() {
        return const_cast<CameraComponent&>(GetActiveCameraInternal());
    }

protected:
    virtual const CameraComponent& GetActiveCameraInternal() const = 0;
    void DrawMainView();

    static std::shared_ptr<CameraComponent> CreateDefaultCamera2D();
    static std::shared_ptr<CameraComponent> CreateDefaultCamera3D();

    const TabId m_id;
    EditorLayer& m_editor;
    Viewer& m_viewer;
    std::string m_title;
    Guid m_guid;

    // @NOTE: each tab should has its own
    // * Undo Stack
    // * Camera
    // * Scene Ref
    // * Associated Asset
    // * Render Target (optional)

    UndoStack m_undo_stack;
};

}  // namespace cave