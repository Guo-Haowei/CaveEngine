#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/string/StringId.h"
#include "cave/runtime/scene/ISceneSystem.h"

namespace cave {

class ViewManager;

struct UIButtonClicked {
    SceneId scene_id;
    StringId event;
    ecs::Entity source;
};

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

    // Vector<UIButtonClicked> m_events;
};

}  // namespace cave
