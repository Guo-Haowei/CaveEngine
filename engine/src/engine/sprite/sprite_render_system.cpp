#include "sprite_render_system.h"

#include "engine/scene/scene.h"
#include "engine/sprite/sprite_renderer.h"

namespace cave {

void RunSpriteRenderSystem(Scene* p_scene, FrameData& p_framedata) {
    if (!p_scene) {
        return;
    }

    auto view = p_scene->View<SpriteRenderer>();

    unused(p_scene);
    unused(p_framedata);
}

}  // namespace cave
