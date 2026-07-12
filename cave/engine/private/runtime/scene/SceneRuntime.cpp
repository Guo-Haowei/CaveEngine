#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/scene/SystemManager.h"
#include "cave/runtime/script/native/NativeScriptSystem.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/ecs/components/All.h"

namespace cave {

SceneRuntime::SceneRuntime(SceneTickDomain domain,
                           RuntimeServices& services,
                           Scene& scene)
    : m_services(services)
    , m_scene(scene)
    , m_query(scene) {

    SceneFeature features = SceneFeature::NativeScript;
    if (domain == SceneTickDomain::Simulate) {
        if (scene.count<MotorComponent>()) {
            features |= SceneFeature::Motor;
        }
        if (scene.count<TileMapInstanceComponent>()) {
            features |= SceneFeature::TileWorld;
        }
    }
    m_features = features;
}

void SceneRuntime::start() {
    if ((int)(m_features & SceneFeature::NativeScript)) {
        m_systems.add<NativeScriptSystem>(*this);
        auto native_scripts = m_systems.get<NativeScriptSystem>();
        native_scripts->alwaysRun();
    }
    if ((int)(m_features & SceneFeature::Motor)) {
        m_systems.add<MotorSystem>(*this);
    }
    if ((int)(m_features & SceneFeature::TileWorld)) {
        m_systems.add<TileWorldSystem>(*this);
    }

    m_systems.start();
}

void SceneRuntime::shutdown() {
    m_systems.shutdown();
}

void SceneRuntime::update(SceneTickContext& ctx) {
    m_systems.update(ctx);
}

}  // namespace cave
