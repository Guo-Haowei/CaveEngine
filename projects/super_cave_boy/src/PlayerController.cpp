#include "PlayerController.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using ::cave::ecs::Entity;

void PlayerController::onCreate(IHostServices& host) {
    unused(host);
}

void PlayerController::onDestroy(IHostServices& host) {
    unused(host);
}

void PlayerController::onUpdate(IHostServices& host, const FrameTime& time) {
    unused(time);

    const SceneQuery& query = host.sceneQuery();
    Entity ent = query.findFirstByName("player");

    auto transform = static_cast<const TransformComponent*>(query.component(TransformComponent_Id, ent));
    Vector3f pos = transform->GetTranslation();
    pos.x += 0.01f;

    SceneCommandWriter& writer = host.sceneWriter();
    writer.SetProperty(ent, TransformComponent_Id, "translation"_sid, pos);
}

}  // namespace super_cave_boy

#if 0
function Player.new(id)
    local self = GameObject.new(id)
    setmetatable(self, Player)
    Log.ok('hello from player.lua')
    self.velocity = g_scene:get_velocity(self.id)
    self.transform = g_scene:get_transform(self.id)
    self.animator_id = g_scene:find_entity_by_name("player_animator_node")
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
