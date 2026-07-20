#pragma once

namespace cave {

class ICanvas;
class Scene;
struct ResolvedView;

struct SceneSubmitContext {
    ICanvas& canvas;
    float dt;
};

void SubmitScene(const ResolvedView& view,
                 const SceneSubmitContext& ctx);

}  // namespace cave
