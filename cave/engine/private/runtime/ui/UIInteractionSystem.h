#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/string/StringId.h"
#include "cave/runtime/scene/ISceneSystem.h"

namespace cave {

class ViewManager;

class UIInteractionSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::UI)

public:
    UIInteractionSystem(SceneRuntime& runtime);
    ~UIInteractionSystem() override;

private:
    void start() override {}
    void update(SceneTickContext& ctx) override;

    DebugId debugId() const override { return m_debug_id; }

    SceneTickDomain domain() const override { return SceneTickDomain::Simulate; }

private:
    const DebugId m_debug_id;
};

}  // namespace cave
