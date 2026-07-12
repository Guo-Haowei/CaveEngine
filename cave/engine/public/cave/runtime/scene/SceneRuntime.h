// =============================================================================
// File: cave/runtime/scene/SceneRuntime.h
// =============================================================================
#pragma once
#include "cave/core/typedefs.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SystemManager.h"

namespace cave {

enum class SceneFeature : uint32_t {
    NativeScript = 1,
    Motor = 2,
    TileWorld = 3,
    All = NativeScript | Motor | TileWorld,
};

DEFINE_ENUM_BITWISE_OPERATIONS(SceneFeature);

class SceneRuntime {
public:
    SceneRuntime(SceneFeature features)
        : m_features(features) {}

    void start(SceneContext& ctx);
    void shutdown();

    void update(SceneTickContext& ctx);

    SystemManager& systems() { return m_systems; }
    const SystemManager& systems() const { return m_systems; }

private:
    SceneContext& context();

    const SceneFeature m_features;
    SystemManager m_systems;

    char m_context[sizeof(SceneContext)]{};
};

}  // namespace cave
