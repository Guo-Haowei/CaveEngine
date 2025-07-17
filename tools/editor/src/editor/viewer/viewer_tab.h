#pragma once
#include "editor/enums.h"
#include "editor/undo_redo/undo_stack.h"

namespace cave {

class CameraComponent;
class EditorLayer;
class Guid;
class InputEvent;
class Scene;

class ViewerTab {
public:
    ViewerTab(EditorLayer& p_editor)
        : m_editor(p_editor) {}

    virtual ~ViewerTab() = default;

    virtual const char* GetName() const = 0;
    virtual const std::string& GetTile() const = 0;

    virtual bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event) = 0;

    virtual void OnEnter(const Guid&) {
        m_undo_stack.Clear();
    }

    virtual void OnExit() {}

    virtual void Update(Scene* p_scene) = 0;

    virtual bool Is2D() const = 0;

    ToolCameraPolicy GetCameraPolicy() const { return m_policy; }

    auto& GetUndoStack() { return m_undo_stack; }

protected:
    ToolCameraPolicy m_policy{ ToolCameraPolicy::Any };
    EditorLayer& m_editor;

    UndoStack m_undo_stack;
};

}  // namespace cave