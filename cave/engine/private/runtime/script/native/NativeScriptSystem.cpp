#pragma once
#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/script/native/NativeScriptRegistry.h"
#include "cave/runtime/script/native/NativeScriptSystem.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace ::cave::ecs;

NativeScriptSystem::NativeScriptSystem()
    : debug_id_(MakeDebugId(this)) {
}

void NativeScriptSystem::ensureCreated(Entity entity,
                                       NativeScriptComponent& component) {
    if (component.created) {
        return;
    }

    if (component.name.empty()) {
        return;
    }

    SceneContext& ctx = context();
    NativeScript* script = ctx.native_scripts.create(component.name);
    if (!script) {
        LOG_ERROR(LogChannel::Script, "Failed to create native script '{}'", component.name.c_str());
        return;
    }

    component.instance = script;
    component.instance->bind(ctx, entity);
    component.instance->onCreate();
    component.created = true;
    component.pending_reload = false;
}

void NativeScriptSystem::destroyScript(NativeScriptComponent& component) {
    if (!component.instance) {
        component.created = false;
        return;
    }

    if (component.created) {
        component.instance->onDestroy();
    }

    component.instance->unbind();

    context().native_scripts.destroy(component.name, component.instance);

    component.instance = nullptr;
    component.created = false;
    component.pending_reload = false;
}

void NativeScriptSystem::reloadIfNeeded(
    Entity entity,
    NativeScriptComponent& component) {
    if (!component.pending_reload) {
        return;
    }

    destroyScript(component);
    ensureCreated(entity, component);
}

void NativeScriptSystem::update(float dt) {
    auto& scene = context().scene;

    for (auto [ent, script] : scene.view<NativeScriptComponent>()) {
        reloadIfNeeded(ent, script);
        ensureCreated(ent, script);

        if (script.instance) {
            script.instance->onUpdate(dt);
        }
    }
}

void NativeScriptSystem::onDetach() {
    auto& scene = context().scene;

    for (auto [ent, script] : scene.view<NativeScriptComponent>()) {
        destroyScript(script);
    }
}

}  // namespace cave
