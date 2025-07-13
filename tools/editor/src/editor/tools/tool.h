#pragma once
#include "editor/enums.h"

namespace my {

class CameraComponent;
class EditorLayer;
class Guid;
class InputEvent;
class Scene;

class ITool {
public:
    ITool(EditorLayer& p_editor)
        : m_editor(p_editor) {}

    virtual ~ITool() = default;

    virtual const char* GetName() const = 0;
    virtual const std::string& GetTile() const = 0;

    virtual bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event) = 0;

    virtual void OnEnter(const Guid&) {}
    virtual void OnExit() {}

    virtual void Update(Scene* p_scene) = 0;

    virtual bool Is2D() const = 0;

    ToolCameraPolicy GetCameraPolicy() const { return m_policy; }

protected:
    ToolCameraPolicy m_policy{ ToolCameraPolicy::Any };
    EditorLayer& m_editor;
};

}  // namespace my
