#pragma once
#include "editor/scene_view/ISceneViewTool.h"

namespace cave {

class TilePaintTool : public ISceneViewTool {
public:
    using ISceneViewTool::ISceneViewTool;

    ~TilePaintTool() override = default;

    void onInputEvents(const InputFrame& input) override;

    void draw(const math::FloatRect& rect) override;
};

}  // namespace cave
