#pragma once
#include "cave/core/ids/GenIdRegistry.h"
#include "cave/runtime/scene/ISceneSystem.h"
#include "cave/runtime/script/native/NativeScript.h"
#include "cave/runtime/script/native/NativeScriptComponent.h"

namespace cave {

class NativeScriptStorage;

class NativeScriptSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::NativeScript)

public:
    NativeScriptSystem(NativeScriptRegistry& script_registry);
    ~NativeScriptSystem() override;

    NativeScript* resolveScript(NativeScriptId id);

    void destroyScript(NativeScriptComponent& component);

    void alwaysRun(SceneContext& ctx);

private:
    void start(SceneContext& ctx) override;
    void update(SceneTickContext& ctx) override;

    DebugId debugId() const override { return m_debug_id; }

    SceneTickDomain domain() const override { return SceneTickDomain::Simulate; }

    void ensureBound(ecs::Entity entity,
                     NativeScriptComponent& component);

    std::unique_ptr<NativeScriptStorage> m_storage;
    const DebugId m_debug_id;
};

}  // namespace cave
