#include "PIESession.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/game/IGameModule.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneContext.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/SceneScheduler.h"
#include "engine/private/runtime/input/InputService.h"

namespace cave {

PIESession::PIESession(EngineServices& services)
    : services_(services)
    , debug_id_(MakeDebugId(this)) {
}

bool PIESession::ensureGameModuleLoaded() {
    if (game_module_) {
        return true;
    }

    if (!game_module_handle_.loadFromDll(start_desc_.game_dll.c_str(), services_.nativeScripts())) {
        return false;
    }

    game_module_ = game_module_handle_.get();

    return game_module_ != nullptr;
}

bool PIESession::start(PIEStartDesc start_desc) {
    DEV_ASSERT(running_ == false);

    start_desc_ = std::move(start_desc);

    if (!ensureGameModuleLoaded()) return false;

    Scene* scene = services_.sceneRegistry().resolve(start_desc_.edit_scene);
    if (!scene) return false;

    if (game_module_) {
        PIEHostServices host(services_, *scene, {});

        game_module_->onModuleLoaded(host);
        host.flushSceneCommands();
    }
    return true;
}

void PIESession::stop() {
    if (running_) {
        onSimEnd();
        running_ = false;
    }

    game_module_handle_.unload();
    game_module_ = nullptr;
}

void PIESession::onSimBegin(SceneId scene_id, ViewId view_id) {
    SceneRegistry& scene_reg = services_.sceneRegistry();

    pie_scene_ = scene_reg.cloneScene(scene_id);

    Scene* scene = scene_reg.resolve(pie_scene_);
    DEV_ASSERT(scene);

    SceneContext ctx = {
        .native_scripts = services_.nativeScripts(),
        .scene = *scene,
        .scene_owner = *this,
        .query = SceneQuery(*scene),
        .engine_services = services_,
    };

    scene->onSimBegin(ctx);

    services_.sceneScheduler().add(this);

    if (game_module_) {
        host_ = std::make_unique<PIEHostServices>(services_, *scene, view_id);
        game_module_->onGameBegin(*host_);
        host_->flushSceneCommands();
    }

    running_ = true;
}

void PIESession::onSimEnd() {
    SceneRegistry& scene_reg = services_.sceneRegistry();

    running_ = false;

    if (Scene* scene = scene_reg.resolve(pie_scene_)) {
        SceneContext ctx = {
            .native_scripts = services_.nativeScripts(),
            .scene = *scene,
            .scene_owner = *this,
            .query = SceneQuery(*scene),
            .engine_services = services_,
        };

        scene->onSimEnd(ctx);

        if (game_module_) {
            game_module_->onGameEnd(*host_);
        }
    }

    services_.sceneScheduler().remove(this);

    scene_reg.destroyScene(pie_scene_);
    pie_scene_ = {};
}

void PIESession::collectSceneTicks(std::vector<SceneTickRequest>& out_requests) {
    out_requests.push_back({ SceneTickMode::Simulation, pie_scene_, *this });
}

void PIESession::commitSceneChange() {
    if (pending_change_.is_none()) return;

    std::string path = std::move(pending_change_.unwrap_unchecked());
    pending_change_ = None();

    unused(path);
    CRASH_NOW();
}

void PIESession::tick(const FrameTime& time) {
    if (!running_ || !game_module_) {
        return;
    }

    SceneRegistry& scene_reg = services_.sceneRegistry();
    Scene* scene = scene_reg.resolve(pie_scene_);
    if (!scene) return;

    game_module_->tick(*host_, time);
    host_->flushSceneCommands();
}

}  // namespace cave
