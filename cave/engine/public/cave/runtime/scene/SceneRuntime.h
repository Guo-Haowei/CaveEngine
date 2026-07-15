// =============================================================================
// File: cave/runtime/scene/SceneRuntime.h
// =============================================================================
#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/typedefs.h"
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/game/MessageBus.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/scene/SystemManager.h"

namespace cave {

class GameSession;
class ISceneTransitionRequests;
class Scene;

enum class SceneFeature : uint32_t {
    NativeScript = 1,
    Motor = 2,
    TileWorld = 4,
    UI = 8,

    All = NativeScript | Motor | TileWorld | UI,
};

DEFINE_ENUM_BITWISE_OPERATIONS(SceneFeature);

class SceneRuntime : public NonCopyable {
public:
    SceneRuntime(SceneTickDomain domain,
                 RuntimeServices& services,
                 Scene& scene,
                 ViewId view_id,
                 GameSession* session = nullptr,
                 ISceneTransitionRequests* transition = nullptr);

    void start(bool editor);
    void shutdown();

    void update(SceneTickContext& ctx);

    Scene& scene() { return m_scene; }
    RuntimeServices& services() { return m_services; }
    GameSession& session() { return *m_session; }
    MessageBus& messageBus() { return m_message_bus; }

    SceneQuery& query() { return m_query; }
    const SceneQuery& query() const { return m_query; }

    template<typename T>
    T* system() { return m_systems.get<T>(); }
    template<typename T>
    const T* system() const { return m_systems.get<T>(); }

    ViewId viewId() const { return m_view_id; }
    ISceneTransitionRequests* transition() const { return m_transition; }

private:
    RuntimeServices& m_services;
    Scene& m_scene;
    SceneQuery m_query;
    ViewId m_view_id;
    GameSession* m_session{};
    ISceneTransitionRequests* m_transition{};

    SceneFeature m_features;
    SystemManager m_systems;
    MessageBus m_message_bus;
};

}  // namespace cave
