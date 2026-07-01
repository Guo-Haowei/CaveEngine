#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/script/native/NativeScriptRegistry.h"
#include "cave/runtime/script/native/NativeScriptSystem.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace ::cave::ecs;

NativeScriptSystem::NativeScriptSystem()
    : debug_id_(MakeDebugId(this)) {
}

void NativeScriptSystem::ensureCreated(SceneContext& ctx,
                                       Entity entity,
                                       NativeScriptComponent& component) {
    if (component.created) {
        return;
    }

    if (component.name.empty()) {
        return;
    }

    NativeScript* script = ctx.native_scripts.create(component.name);
    if (!script) {
        LOG_ERROR(LogChannel::Script, "Failed to create native script '{}'", component.name.c_str());
        return;
    }

    component.instance = script;
    component.instance->bind(entity);
    component.instance->onCreate(ctx);
    component.created = true;
    component.pending_reload = false;
}

void NativeScriptSystem::destroyScript(NativeScriptRegistry& script_registry,
                                       NativeScriptComponent& component) {
    if (!component.instance) {
        component.created = false;
        return;
    }

    if (component.created) {
        component.instance->onDestroy();
    }

    component.instance->unbind();

    script_registry.destroy(component.name, component.instance);

    component.instance = nullptr;
    component.created = false;
    component.pending_reload = false;
}

void NativeScriptSystem::reloadIfNeeded(SceneContext& ctx,
                                        Entity entity,
                                        NativeScriptComponent& component) {
    if (!component.pending_reload) {
        return;
    }

    destroyScript(ctx.native_scripts, component);
    ensureCreated(ctx, entity, component);
}

void NativeScriptSystem::update(SceneTickContext& ctx) {
    auto& scene = ctx.scene_ctx.scene;

    for (auto [ent, script] : scene.view<NativeScriptComponent>()) {
        reloadIfNeeded(ctx.scene_ctx, ent, script);
        ensureCreated(ctx.scene_ctx, ent, script);

        if (script.instance) {
            script.instance->onUpdate(ctx.scene_ctx, ctx.dt);
        }
    }
}

void NativeScriptSystem::onDetach(SceneContext& ctx) {
    auto& scene = ctx.scene;

    for (auto [ent, script] : scene.view<NativeScriptComponent>()) {
        destroyScript(ctx.native_scripts, script);
    }
}

}  // namespace cave
