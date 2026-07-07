#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/ids/GenIdRegistry.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/script/native/NativeScriptSystem.h"
#include "cave/runtime/script/native/NativeScriptRegistry.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"

namespace cave {

using namespace ::cave::ecs;

struct NativeScriptDeleter {
    NativeScriptDestroyFn destroy = nullptr;

    void operator()(NativeScript* p) const {
        if (p && destroy) {
            destroy(p);
        }
    }
};

using NativeScriptPtr = std::unique_ptr<NativeScript, NativeScriptDeleter>;

class NativeScriptStorage : private GenIdRegistry<NativeScript, NativeScriptPtr> {
    using Base = GenIdRegistry<NativeScript, NativeScriptPtr>;

public:
    NativeScriptStorage(NativeScriptRegistry& registry)
        : m_registry(registry)
        , m_manager_id(nextManagerId()) {}

    NativeScriptId createScript(std::string_view script_id) {
        const NativeScriptInfo* info = m_registry.find(script_id);
        if (!info) {
            return {};
        }

        NativeScript* script = info->create();
        if (!script) {
            return {};
        }

        NativeScriptPtr ptr{ script, NativeScriptDeleter{ info->destroy } };

        auto local_id = Base::create(std::move(ptr));

        return NativeScriptId{
            .manager_id = m_manager_id,
            .local_id = local_id,
        };
    }

    bool destroyScript(NativeScriptId id) {
        if (DEV_VERIFY(fromThisManager(id))) {
            Base::destroy(id.local_id);
            return true;
        }
        return false;
    }

    NativeScript* resolveScript(NativeScriptId id) {
        return fromThisManager(id) ? Base::resolve(id.local_id) : nullptr;
    }

private:
    bool fromThisManager(NativeScriptId id) const {
        return id.manager_id == m_manager_id;
    }

    static uint32_t nextManagerId() {
        static std::atomic<uint32_t> next{ 1 };
        return next.fetch_add(1, std::memory_order_relaxed);
    }

    NativeScriptRegistry& m_registry;
    const uint32_t m_manager_id;
};

NativeScriptSystem::NativeScriptSystem(NativeScriptRegistry& script_registry)
    : m_storage(std::make_unique<NativeScriptStorage>(script_registry))
    , m_debug_id(MakeDebugId(this)) {
}

NativeScriptSystem::~NativeScriptSystem() = default;

void NativeScriptSystem::ensureBound(Entity entity,
                                     NativeScriptComponent& script) {
    if (script.name.empty()) {
        return;
    }

    if (script.handle.valid()) {
        if (m_storage->resolveScript(script.handle)) {
            return;
        }

        script.handle = {};
        LOG_WARN(LogChannel::Script, "Found stale handle '{}'", script.name.c_str());
    }

    NativeScriptId instance_id = m_storage->createScript(script.name);
    NativeScript* instance = m_storage->resolveScript(instance_id);
    if (!instance) {
        LOG_ERROR(LogChannel::Script, "Failed to create native script '{}'", script.name.c_str());
        return;
    }

    instance->bind(entity, script.params);

    script.handle = instance_id;
}

NativeScript* NativeScriptSystem::resolveScript(NativeScriptId id) {
    return m_storage->resolveScript(id);
}

void NativeScriptSystem::destroyScript(NativeScriptComponent& script) {
    NativeScript* instance = m_storage->resolveScript(script.handle);

    if (instance) {
        instance->destroy();
        instance->unbind();
        m_storage->destroyScript(script.handle);
    }

    script.handle = {};
}

void NativeScriptSystem::alwaysRun(SceneContext& ctx) {
    auto& scene = ctx.scene;

    SceneCommandWriter writer(ctx.services.assetRegistry());

    for (auto [ent, script] : scene.view<NativeScriptComponent>()) {
        ensureBound(ent, script);

        NativeScript* instance = m_storage->resolveScript(script.handle);
        if (DEV_VERIFY(instance)) {
            instance->alwaysRun(ctx, writer);
        }
    }

    SceneCommandExecutor executor(scene);
    EntityMap map(writer.allocationCount());
    SceneCommandPlayback::Play(writer, executor, { map, scene });
}

void NativeScriptSystem::start(SceneContext& ctx) {
    for (auto [ent, script] : ctx.scene.view<NativeScriptComponent>()) {
        if (NativeScript* instance = m_storage->resolveScript(script.handle)) {
            instance->start(ctx);
        }
    }
}

void NativeScriptSystem::update(SceneTickContext& ctx) {
    auto& scene = ctx.scene_ctx.scene;
    for (auto [ent, script] : scene.view<NativeScriptComponent>()) {
        if (NativeScript* instance = m_storage->resolveScript(script.handle)) {
            instance->update(ctx.scene_ctx, ctx.dt);
        }
    }
}

}  // namespace cave
