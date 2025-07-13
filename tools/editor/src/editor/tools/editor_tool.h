#pragma once
#include "tool_interface.h"

#include "engine/input/input_event.h"
#include "engine/scene/scene.h"

namespace my {

class Viewer;

class EditorTool : public ITool {
public:
    EditorTool(EditorLayer& p_editor, Viewer* p_viewer)
        : ITool(p_editor), m_viewer(p_viewer) {}

    bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event) override;

    CameraComponent& GetCamera() override;

    void OnEnter() override;
    void OnExit() override;

    void Draw(Scene* p_scene) override;

    virtual bool Is2D() const { return false; }

    const char* GetName() const override { return "Editor"; }

protected:
    enum class GizmoState {
        Translating,
        Rotating,
        Scaling,
    } m_state{ GizmoState::Translating };

    Viewer* m_viewer;

    CameraComponent m_camera;
};

}  // namespace my
