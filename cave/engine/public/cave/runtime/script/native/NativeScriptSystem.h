// =============================================================================
// File: cave/runtime/script/native/NativeScriptSystem.h
// =============================================================================
#pragma once
#include "cave/runtime/scene/ISceneSystem.h"
#include "cave/runtime/script/native/NativeScript.h"
#include "cave/runtime/script/native/NativeScriptComponent.h"

namespace cave {

class NativeScriptSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::NativeScript)

public:
    NativeScriptSystem(NativeScriptRegistry& script_registry);
    ~NativeScriptSystem() override { clear(); }

    void destroyScript(NativeScriptRegistry& script_registry,
                       NativeScriptComponent& component);

    void alwaysRun(SceneContext& ctx);

private:
    void clear();

    void start(SceneContext& ctx) override;
    void update(SceneTickContext& ctx) override;

    DebugId debugId() const override { return m_debug_id; }

    SceneTickDomain domain() const override {
        return SceneTickDomain::Simulate | SceneTickDomain::Editor;
    }

    void ensureBound(SceneContext& ctx,
                     ecs::Entity entity,
                     NativeScriptComponent& component);
    void reloadIfNeeded(SceneContext& ctx,
                        ecs::Entity entity,
                        NativeScriptComponent& component);

    NativeScriptRegistry& m_script_registry;
    const DebugId m_debug_id;

    std::unordered_map<NativeScript*, FixedString<32>> m_scripts;
};

}  // namespace cave
