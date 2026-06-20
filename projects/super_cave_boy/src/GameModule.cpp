#include "GameModule.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/display/IDebugDrawService.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

#include "CameraController.h"
#include "PlayerController.h"

namespace super_cave_boy {

using namespace ::cave;
using namespace ::cave::math;

GameModule::GameModule() = default;
GameModule::~GameModule() = default;

void GameModule::registerNativeScripts(NativeScriptRegistry& registry) {
    registry.registerScript<PlayerController>("PlayerController");
    registry.registerScript<CameraController>("CameraController");
}

void GameModule::onModuleLoaded(IHostServices& host) {
    LOG_OK(LogChannel::Game, "GameModule Loaded");

    unused(host);
}

void GameModule::onModuleUnloaded(IHostServices&) {
}

void GameModule::onGameBegin(IHostServices& host) {
    unused(host);
}

void GameModule::onGameEnd(IHostServices& host) {
    unused(host);
}

void GameModule::tick(IHostServices& host, const FrameTime& time) {
    unused(time);

    IDebugDrawService& debug_draw = host.debugDraw();
    SceneQuery& query = host.sceneQuery();

    const TileWorldSystem* tile_world = query.system<TileWorldSystem>();

    ecs::Entity ent = query.findFirstByName("player");

    const auto transform = query.component<TransformComponent>(ent);
    const auto collider = query.component<ColliderComponent>(ent);
    const Mat4f& m = transform->GetWorldMatrix();
    const Shape& shape = collider->shape();
    Vec2f min = Vec2f::Zero - Vec2f(shape.data.half.xy);
    Vec2f max = Vec2f::Zero + Vec2f(shape.data.half.xy);

    debug_draw.addBox2Frame(min, max, Vec4f::One, &m, 0.04f);

    unused(tile_world);
}

}  // namespace super_cave_boy
