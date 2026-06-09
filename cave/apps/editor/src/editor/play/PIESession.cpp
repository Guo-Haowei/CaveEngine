#include "PIESession.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/game/IGameModule.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/framework/IScriptService.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/SceneScheduler.h"

namespace cave {

PIESession::PIESession(IApplication& app)
    : app_(app)
    , debug_id_(MakeDebugId(this)) {
}

bool PIESession::ensureGameModuleLoaded() {
    if (game_module_) {
        return true;
    }

    if (!game_module_handle_.LoadFromDll(start_desc_.game_dll.c_str())) {
        return false;
    }

    game_module_ = game_module_handle_.Get();
    return game_module_ != nullptr;
}

bool PIESession::start(PIEStartDesc start_desc) {
    DEV_ASSERT(running_ == false);

    start_desc_ = std::move(start_desc);

    if (!ensureGameModuleLoaded()) return false;

    SceneRegistry* reg = app_.GetSceneRegistry();
    if (!reg) return false;

    Scene* scene = reg->Resolve(start_desc_.edit_scene);
    if (!scene) return false;

    PIEHostServices host(app_, *scene, {});

    game_module_->onModuleLoaded(host);
    host.flushSceneCommands();
    return true;
}

void PIESession::stop() {
    if (running_) {
        onSimEnd();
        running_ = false;
    }

    game_module_handle_.Unload();
    game_module_ = nullptr;
}

void PIESession::onSimBegin(SceneId scene_id, ViewId view_id) {
    SceneRegistry& scene_manager = *app_.GetSceneRegistry();

    pie_scene_ = scene_manager.Clone(scene_id);

    Scene* scene = scene_manager.Resolve(pie_scene_);
    DEV_ASSERT(scene);
    app_.ScriptService()->OnSimBegin(*scene);

    app_.GetSceneScheduler().Register(this);

    host_ = std::make_unique<PIEHostServices>(app_, *scene, view_id);
    game_module_->onGameBegin(*host_);
    host_->flushSceneCommands();

    running_ = true;
}

void PIESession::onSimEnd() {
    SceneRegistry& scene_manager = *app_.GetSceneRegistry();

    running_ = false;

    if (Scene* scene = app_.GetSceneRegistry()->Resolve(pie_scene_)) {
        app_.ScriptService()->OnSimEnd();

        game_module_->onGameEnd(*host_);
    }

    app_.GetSceneScheduler().Unregister(this);

    scene_manager.Destroy(pie_scene_);
    pie_scene_ = {};
}

void PIESession::CollectSceneTicks(std::vector<SceneTickRequest>& out_requests) {
    out_requests.push_back({ SceneTickMode::Simulation, pie_scene_ });
}

void PIESession::tick(const FrameTime& time) {
    if (!running_ || !game_module_) {
        return;
    }

    SceneRegistry* reg = app_.GetSceneRegistry();
    Scene* scene = reg->Resolve(pie_scene_);
    if (!scene) return;

    game_module_->tick(*host_, time);
    host_->flushSceneCommands();
}

}  // namespace cave
