#include "PlayerController.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/TileMapRendererComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using ::cave::ecs::Entity;

void PlayerController::onCreate(IHostServices& host) {
    initLevel(host);

    const SceneQuery& query = host.sceneQuery();
    player_ = query.findFirstByName("player");
    player_animator_ = query.findFirstByName("player_animator_node");
}

void PlayerController::onDestroy(IHostServices& host) {
    unused(host);
}

void PlayerController::onUpdate(IHostServices& host, const FrameTime& time) {
    const IGameInput& input = host.gameInput();
    SceneQuery& query = host.sceneQuery();

    const int move_x = (int)input.isPressed("ui_right"_sid) - (int)input.isPressed("ui_left"_sid);

    auto animator = static_cast<SpriteAnimatorComponent*>(query.component(SpriteAnimatorComponent_Id, player_animator_));
    DEV_ASSERT(animator);

    if (move_x == 0) {
        animator->SetClip("idle");
    } else {
        animator->SetClip("walk");

        auto transform = static_cast<TransformComponent*>(query.component(TransformComponent_Id, player_));

        const float x_speed = 4.0f;
        const float dx = x_speed * time.dt * move_x;
        transform->IncreaseTranslation(Vector3f(dx, 0.0f, 0.0f));

        Vector4f rotation = move_x < 0 ? Vector4f{ 0.0f, 1.0f, 0.0f, 0.0f } : Vector4f{ 0.0f, 0.0f, 0.0f, 1.0f };
        transform->SetRotation(rotation);
    }
}

void PlayerController::initLevel(cave::IHostServices& host) {
    unused(host);
    // const SceneQuery& query = host.sceneQuery();
    // auto instance = static_cast<const TileMapRendererComponent*>(query.component(TileMapRendererComponent_Id, player_ent_));
    // const TileMapAsset* tile_map = instance->GetTileMapHandle().Get();
    // unused(tile_map);
}

}  // namespace super_cave_boy

#if 0
function Player.new(id)
    local self = GameObject.new(id)
    setmetatable(self, Player)
    Log.ok('hello from player.lua')
    self.velocity = g_scene:get_velocity(self.id)
    self.transform = g_scene:get_transform(self.id)
    self.animator = g_scene:get_animator(self.animator_id)
    return self
end

function Player:_process(timestep)
    local move_x = Input.is_action_pressed('ui_right') - Input.is_action_pressed('ui_left')
    local jump = Input.is_action_just_pressed('ui_up')
    if jump ~= 0 then
        self.velocity.linear.y = 10
    end

    if move_x == 0 then
        self.animator:set_clip('idle')
        self.velocity.linear.x = 0
    else
        self.animator:set_clip('walk')

        -- @TODO: attach sprite as child to player
        local rotate_z = move_x < 0 and math.rad(180) or 0
        local euler = Vector3(0, rotate_z, 0)
        self.transform:set_rotation(Quaternion(euler))

        self.velocity.linear.x = move_x * 3.5
    end
end
#endif
