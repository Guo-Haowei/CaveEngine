#pragma once
#include "tab_id.h"

#include "engine/assets/guid.h"
#include "editor/enums.h"
#include "editor/undo_redo/undo_stack.h"

namespace cave {

class EditorCameraController;
class TabId;

class ViewerTab {
public:
    ViewerTab(EditorLayer& p_editor, Viewer& p_viewer);

    virtual ~ViewerTab() = default;

    virtual bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event);

    virtual void OnCreate(const Guid&) {}
    virtual void OnDestroy() {}

    virtual void OnActivate() {}
    virtual void OnDeactivate() {}

    virtual void Draw(Scene*);

    virtual void Update(Scene*) {}

    virtual void UpdateCamera();

    ToolCameraPolicy GetCameraPolicy() const { return m_policy; }

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
    const CameraComponent& GetActiveCameraInternal() const;

    const TabId m_id;
    EditorLayer& m_editor;
    Viewer& m_viewer;
    std::string m_title;
    Guid m_guid;

    // @TODO: actually own the camera and controller
    ToolCameraPolicy m_policy{ ToolCameraPolicy::Any };

    // @NOTE: each tab should has its own
    // * Undo Stack
    // * Camera
    // * Scene Ref
    // * Associated Asset
    // * Render Target (optional)

    UndoStack m_undo_stack;

    std::shared_ptr<EditorCameraController> m_controller;
};

}  // namespace cave