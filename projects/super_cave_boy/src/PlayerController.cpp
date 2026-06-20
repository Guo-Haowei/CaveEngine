#include "PlayerController.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using ::cave::ecs::Entity;

void PlayerController::onCreate() {
    const SceneQuery query(context().scene);

    animator_ = query.findFirstByName("player_animator_node");
}

void PlayerController::onDestroy() {
}

void PlayerController::onUpdate(float dt) {
    const IGameInput& input = context().game_input;
    SceneQuery query(context().scene);

    const int move_x = (int)input.isPressed("ui_right"_sid) - (int)input.isPressed("ui_left"_sid);
    const int move_y = (int)input.isPressed("ui_up"_sid) - (int)input.isPressed("ui_down"_sid);

    auto animator = query.component<SpriteAnimatorComponent>(animator_);
    DEV_ASSERT(animator);

    if (move_x == 0 && move_y == 0) {
        animator->SetClip("idle");
    } else {
        animator->SetClip("walk");

        auto transform = query.component<TransformComponent>(entity());

        const float x_speed = 4.0f;
        const float dx = x_speed * dt * move_x;
        const float dy = x_speed * dt * move_y;

        transform->IncreaseTranslation(Vec3f(dx, dy, 0.0f));

        Vec4f rotation = move_x < 0 ? Vec4f{ 0.0f, 1.0f, 0.0f, 0.0f } : Vec4f{ 0.0f, 0.0f, 0.0f, 1.0f };
        transform->SetRotation(rotation);
    }
}

}  // namespace super_cave_boy
