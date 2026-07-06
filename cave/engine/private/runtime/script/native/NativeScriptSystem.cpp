#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/script/native/NativeScriptRegistry.h"
#include "cave/runtime/script/native/NativeScriptSystem.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"

namespace cave {

using namespace ::cave::ecs;

NativeScriptSystem::NativeScriptSystem(NativeScriptRegistry& script_registry)
    : m_script_registry(script_registry)
    , m_debug_id(MakeDebugId(this)) {
}

void NativeScriptSystem::ensureBound(SceneContext& ctx,
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
    component.instance->bind(entity, component.params);
    component.created = true;
    component.always_run_called = false;
    component.pending_reload = false;

    ++m_num_instance;
}

void NativeScriptSystem::destroyScript(NativeScriptRegistry& script_registry,
                                       NativeScriptComponent& component) {
    if (!component.instance) {
        component.created = false;
        return;
    }

    if (component.created) {
        component.instance->destroy();
    }

    component.instance->unbind();

    script_registry.destroy(component.name, component.instance);

    component.instance = nullptr;
    component.created = false;
    component.pending_reload = false;

    --m_num_instance;
}

void NativeScriptSystem::reloadIfNeeded(SceneContext& ctx,
                                        Entity entity,
                                        NativeScriptComponent& component) {
    if (!component.pending_reload) {
        return;
    }

    destroyScript(ctx.native_scripts, component);
    ensureBound(ctx, entity, component);
}

void NativeScriptSystem::alwaysRun(SceneContext& ctx) {
    auto& scene = ctx.scene;

    SceneCommandWriter writer(ctx.engine_services.assetRegistry());

    for (auto [ent, script] : scene.view<NativeScriptComponent>()) {
        reloadIfNeeded(ctx, ent, script);
        ensureBound(ctx, ent, script);

        if (script.instance && !script.always_run_called) {
            script.instance->alwaysRun(ctx, writer);
            script.always_run_called = true;
        }
    }

    SceneCommandExecutor executor(scene);
    EntityMap map(writer.allocationCount());
    SceneCommandPlayback::Play(writer, executor, { map, scene });
}

void NativeScriptSystem::start(SceneContext& ctx) {
    for (auto [ent, script] : ctx.scene.view<NativeScriptComponent>()) {
        if (script.instance) {
            script.instance->start(ctx);
        }
    }
}

void NativeScriptSystem::update(SceneTickContext& ctx) {
    auto& scene = ctx.scene_ctx.scene;

    for (auto [ent, script] : scene.view<NativeScriptComponent>()) {
        if (script.instance) {
            script.instance->update(ctx.scene_ctx, ctx.dt);
        }
    }
}

void NativeScriptSystem::clear() {
    if (m_num_instance) {
        LOG_WARN(LogChannel::Script, "{} scripts memory leaked", m_num_instance);
    }
#if 0
    auto& scene = ctx.scene;

    for (auto [ent, script] : scene.view<NativeScriptComponent>()) {
        destroyScript(m_native_scripts, script);
    }
#endif
}

}  // namespace cave
