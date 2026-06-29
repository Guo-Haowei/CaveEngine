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
    NativeScriptSystem();

    void onDetach(SceneContext& ctx) override;

    void update(SceneTickContext& ctx) override;

    DebugId debugId() const override { return debug_id_; }

private:
    void ensureCreated(SceneContext& ctx,
                       ecs::Entity entity,
                       NativeScriptComponent& component);
    void destroyScript(SceneContext& ctx,
                       NativeScriptComponent& component);
    void reloadIfNeeded(SceneContext& ctx,
                        ecs::Entity entity,
                        NativeScriptComponent& component);

    const DebugId debug_id_;
};

}  // namespace cave
