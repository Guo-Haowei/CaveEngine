// =============================================================================
// File: cave/runtime/script/native/NativeScriptSystem.h
// =============================================================================
#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/runtime/scene/ISceneSystem.h"
#include "cave/runtime/script/native/NativeScript.h"
#include "cave/runtime/script/native/NativeScriptComponent.h"

namespace cave {

class NativeScriptStorage;

class NativeScriptSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::NativeScript)

public:
    NativeScriptSystem(SceneRuntime& runtime);
    ~NativeScriptSystem() override;

    NativeScript* resolveScript(NativeScriptId id);

    void destroyScript(NativeScriptComponent& component);

    void alwaysRun();

private:
    void start() override;
    void update(SceneTickContext& ctx) override;

    DebugId debugId() const override { return m_debug_id; }

    SceneTickDomain domain() const override { return SceneTickDomain::Simulate; }

    void ensureBound(ecs::Entity entity,
                     NativeScriptComponent& component);

    bool m_always_run_called = false;
    Owner<NativeScriptStorage> m_storage;
    const DebugId m_debug_id;
};

}  // namespace cave
