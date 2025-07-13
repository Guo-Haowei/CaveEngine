#pragma once

namespace my {

class CameraComponent;
class EditorLayer;
class InputEvent;
class Scene;

enum class EditorToolType {
    None,
    Edit,
    TileMap,
    Count,
};

class ITool {
public:
    ITool(EditorLayer& p_editor)
        : m_editor(p_editor) {}

    virtual ~ITool() = default;

    virtual const char* GetName() const = 0;

    virtual bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event) = 0;

    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;

    virtual CameraComponent& GetCamera() = 0;
    virtual void Draw(Scene* p_scene) = 0;

    virtual bool Is2D() const = 0;

protected:
    EditorLayer& m_editor;
};

}  // namespace my
