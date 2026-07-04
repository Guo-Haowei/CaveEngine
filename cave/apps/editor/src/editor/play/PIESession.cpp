#include "PIESession.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/game/IGameModule.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneContext.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
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
        endPIESession();
        running_ = false;
    }

    game_module_handle_.unload();
    game_module_ = nullptr;
}

SceneContext PIESession::makeSceneContext(Scene& scene) {
    return SceneContext{
        .native_scripts = services_.nativeScripts(),
        .scene = scene,
        .scene_transition = *this,
        .query = SceneQuery(scene),
        .engine_services = services_,
    };
}

Scene* PIESession::beginPIEScene(Scene* asset_scene) {
    DEV_ASSERT(asset_scene);
    SceneRegistry& scene_reg = services_.sceneRegistry();
    pie_scene_ = scene_reg.cloneScene(*asset_scene);

    Scene* scene = scene_reg.resolve(pie_scene_);
    if (DEV_VERIFY(scene)) {
        SceneContext ctx = makeSceneContext(*scene);
        scene->onSimBegin(ctx);
    }
    return scene;
}

void PIESession::endPIEScene() {
    if (!pie_scene_.isValid()) {
        return;
    }

    SceneRegistry& scene_reg = services_.sceneRegistry();
    Scene* scene = scene_reg.resolve(pie_scene_);

    if (DEV_VERIFY(scene)) {
        SceneContext ctx = makeSceneContext(*scene);
        scene->onSimEnd(ctx);
        scene_reg.destroyScene(pie_scene_);
    }

    pie_scene_ = {};
}

void PIESession::beginPIESession(SceneId scene_id, ViewId view_id) {
    services_.sceneScheduler().add(this);

    SceneRegistry& scene_reg = services_.sceneRegistry();
    Scene* pie_scene = beginPIEScene(scene_reg.resolve(scene_id));
    if (DEV_VERIFY(pie_scene)) {
        if (game_module_) {
            // @BUG: host still points to old scene, after scene change
            // this is dangerous, but it's fine for now
            // because we are going to remove PIEHostServices entirely.
            host_ = std::make_unique<PIEHostServices>(services_, *pie_scene, view_id);
            game_module_->onGameBegin(*host_);
            host_->flushSceneCommands();
        }

        running_ = true;
    }
}

void PIESession::endPIESession() {
    services_.sceneScheduler().remove(this);

    if (!pie_scene_.isValid()) {
        return;
    }

    running_ = false;

    endPIEScene();

    if (game_module_ && host_) {
        game_module_->onGameEnd(*host_);
    }
}

void PIESession::commitSceneChange(std::string&& path) {
    DEV_ASSERT(!path.empty());

    endPIEScene();

    auto handle = services_.assetRegistry().findByPath<Scene>(path);
    if (handle.is_none()) {
        LOG_ERROR(LogChannel::Asset, "Failed to find asset '{}'", path);
        return;
    }

    Scene* asset_scene = handle.unwrap_unchecked().get();
    if (!asset_scene) {
        LOG_ERROR(LogChannel::Asset, "Failed to load asset '{}'", path);
        return;
    }

    beginPIEScene(asset_scene);
}

void PIESession::collectSceneTicks(std::vector<SceneTickRequest>& out_requests) {
    out_requests.push_back({ SceneTickMode::Simulation, pie_scene_, *this });
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
