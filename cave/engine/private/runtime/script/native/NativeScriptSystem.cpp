#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/ids/GenIdRegistry.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneRuntime.h"
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

NativeScriptSystem::NativeScriptSystem(SceneRuntime& runtime)
    : ISceneSystem(runtime)
    , m_storage(MakeOwner<NativeScriptStorage>(runtime.services().nativeScripts()))
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
    }

    NativeScriptId instance_id = m_storage->createScript(script.name);
    NativeScript* instance = m_storage->resolveScript(instance_id);
    if (!instance) {
        LOG_ERROR(LogChannel::Script, "Failed to create native script '{}'", script.name.c_str());
        return;
    }

    instance->bind(&m_runtime, entity, script.params);

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

void NativeScriptSystem::alwaysRun() {
    Scene& scene = m_runtime.scene();

    SceneCommandWriter writer(m_runtime.services().assetRegistry());

    for (auto [ent, script] : scene.view<NativeScriptComponent>()) {
        ensureBound(ent, script);

        NativeScript* instance = m_storage->resolveScript(script.handle);
        if (DEV_VERIFY(instance)) {
            instance->alwaysRun(writer);
        }
    }

    SceneCommandExecutor executor(scene);
    EntityMap map(writer.allocationCount());

    SceneCommandPlayback::Play(writer, executor, { map, scene });
    m_always_run_called = true;
}

void NativeScriptSystem::start() {
    DEV_ASSERT(m_always_run_called);
    for (auto [ent, script] : m_runtime.scene().view<NativeScriptComponent>()) {
        if (NativeScript* instance = m_storage->resolveScript(script.handle)) {
            instance->start();
        }
    }
}

void NativeScriptSystem::update(SceneTickContext& ctx) {
    for (auto [ent, script] : m_runtime.scene().view<NativeScriptComponent>()) {
        if (NativeScript* instance = m_storage->resolveScript(script.handle)) {
            instance->update(ctx.dt);
        }
    }
}

}  // namespace cave
