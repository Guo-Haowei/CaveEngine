#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/scene/SystemManager.h"
#include "cave/runtime/script/native/NativeScriptSystem.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"
#include "cave/runtime/ui/UIComponents.h"

// @TODO: refactor
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/ui/UIInteractionSystem.h"

namespace cave {

SceneRuntime::SceneRuntime(SceneTickDomain domain,
                           RuntimeServices& services,
                           Scene& scene,
                           ViewId view_id,
                           GameSession* session,
                           ISceneTransitionRequests* transition)
    : m_services(services)
    , m_scene(scene)
    , m_query(scene)
    , m_view_id(view_id)
    , m_session(session)
    , m_transition(transition) {

    SceneFeature features = SceneFeature::NativeScript;
    if (domain == SceneTickDomain::Simulate) {
        if (scene.count<UICanvasComponent>()) {
            features |= SceneFeature::UI;
        }
        if (scene.count<MotorComponent>()) {
            features |= SceneFeature::Motor;
        }
        if (scene.count<TileMapLayerComponent>() ||
            scene.count<TileMapInstanceComponent>()) {
            features |= SceneFeature::TileWorld;
        }
    }
    m_features = features;
}

void SceneRuntime::start(bool editor) {
    if ((int)(m_features & SceneFeature::TileWorld)) {
        m_systems.add<TileWorldSystem>(*this);
    }
    if ((int)(m_features & SceneFeature::UI)) {
        m_systems.add<UIInteractionSystem>(*this);
    }
    if ((int)(m_features & SceneFeature::NativeScript)) {
        m_systems.add<NativeScriptSystem>(*this);
        auto native_scripts = m_systems.get<NativeScriptSystem>();
        native_scripts->alwaysRun();
    }
    if ((int)(m_features & SceneFeature::Motor)) {
        m_systems.add<MotorSystem>(*this);
    }

    if (!editor) {
        m_systems.start();
    }
}

void SceneRuntime::shutdown() {
    m_systems.shutdown();
}

void SceneRuntime::update(SceneTickContext& ctx) {
    m_systems.update(ctx);
}

}  // namespace cave
