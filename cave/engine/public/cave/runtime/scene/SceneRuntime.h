// =============================================================================
// File: cave/runtime/scene/SceneRuntime.h
// =============================================================================
#pragma once
#include "cave/core/typedefs.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SystemManager.h"

namespace cave {

class Scene;

enum class SceneFeature : uint32_t {
    NativeScript = 1,
    Motor = 2,
    TileWorld = 3,
    All = NativeScript | Motor | TileWorld,
};

DEFINE_ENUM_BITWISE_OPERATIONS(SceneFeature);

class SceneRuntime {
public:
    SceneRuntime(SceneTickDomain domain,
                 RuntimeServices& services,
                 Scene& scene);

    void start();
    void shutdown();

    void update(SceneTickContext& ctx);

    Scene& scene() { return m_scene; }
    RuntimeServices& services() { return m_services; }

    SceneQuery& query() { return m_query; }
    const SceneQuery& query() const { return m_query; }

    template<typename T>
    T* system() { return m_systems.get<T>(); }
    template<typename T>
    const T* system() const { return m_systems.get<T>(); }

    ViewId view_id;

private:
    RuntimeServices& m_services;
    Scene& m_scene;
    SceneQuery m_query;
    SceneFeature m_features;
    SystemManager m_systems;
};

}  // namespace cave
