#pragma once
#include "tool.h"

#include "engine/input/input_event.h"
#include "engine/scene/scene.h"

namespace my {

class Viewer;

class EditorTool : public ITool {
public:
    EditorTool(EditorLayer& p_editor, Viewer* p_viewer)
        : ITool(p_editor), m_viewer(p_viewer) {}

    bool HandleInput(const std::shared_ptr<InputEvent>& p_input_event) override;

    void OnEnter() override;
    void OnExit() override;

    void Update(Scene* p_scene) override;

    virtual bool Is2D() const { return false; }

    const char* GetName() const override { return "Editor"; }

protected:
    GizmoAction m_state{ GizmoAction::Translate };

    Viewer* m_viewer;
};

}  // namespace my
