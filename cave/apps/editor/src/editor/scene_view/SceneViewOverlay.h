#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/ViewId.h"

namespace cave {

class ICanvas;
class Scene;

class SceneViewOverlay {
public:
    void drawSelectionHighlight(ICanvas& canvas,
                                ViewId view_id,
                                const Scene& scene,
                                ecs::Entity ent);

private:
};

}  // namespace cave
