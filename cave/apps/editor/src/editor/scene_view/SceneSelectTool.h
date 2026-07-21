#pragma once
#include "editor/scene_view/ISceneViewTool.h"

// @TODO: refactor
#include "editor/Enums.h"

namespace cave {

class SceneSelectTool : public ISceneViewTool {
public:
    using ISceneViewTool::ISceneViewTool;

    ~SceneSelectTool() override = default;

    Option<PickData> getPickData(const math::Vec2f&) override;

    void onInputEvents(const InputFrame& input, const WindowState& state) override;

    void draw(const math::FloatRect& rect) override;

private:
    void drawGizmo(const math::FloatRect& rect, bool ortho);

    GizmoAction m_gizmo_action{ GizmoAction::Translate };
};

}  // namespace cave
