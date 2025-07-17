#pragma once
#include "editor/enums.h"
#include "editor/undo_redo/undo_stack.h"

namespace cave {

class CameraComponent;
class EditorLayer;
class Guid;
class InputEvent;
class Scene;
class Viewer;

class ViewerTab {
public:
    ViewerTab(EditorLayer& p_editor, Viewer& p_viewer);

    virtual ~ViewerTab() = default;

    virtual bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event);

    virtual void OnEnter(const Guid&);

    virtual void OnExit() {}

    virtual void Draw(Scene*);

    virtual void Update(Scene*) {}

    virtual bool Is2D() const { return false; }

    ToolCameraPolicy GetCameraPolicy() const { return m_policy; }

    UndoStack& GetUndoStack() { return m_undo_stack; }

    int GetId() const { return m_id; }

    const std::string& GetTitle() const {
        return m_title;
    }

protected:
    const int m_id;
    EditorLayer& m_editor;
    Viewer& m_viewer;
    std::string m_title;

    // @TODO: actually own the camera and controller
    ToolCameraPolicy m_policy{ ToolCameraPolicy::Any };

    UndoStack m_undo_stack;
};

}  // namespace cave