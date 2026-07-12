#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/MotorSystem.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/scene/SystemManager.h"
#include "cave/runtime/script/native/NativeScriptSystem.h"
#include "cave/runtime/tile_map/TileWorldSystem.h"

namespace cave {

void SceneRuntime::start(SceneContext& ctx) {
    std::memcpy(&m_context, &ctx, sizeof(SceneContext));

    if ((int)(m_features & SceneFeature::NativeScript)) {
        m_systems.add<NativeScriptSystem>(ctx.services.nativeScripts());
        auto native_scripts = m_systems.get<NativeScriptSystem>();
        native_scripts->alwaysRun(ctx);
    }
    if ((int)(m_features & SceneFeature::Motor)) {
        m_systems.add<MotorSystem>();
    }
    if ((int)(m_features & SceneFeature::TileWorld)) {
        m_systems.add<TileWorldSystem>();
    }

    m_systems.start(ctx);
}

void SceneRuntime::shutdown() {
    m_systems.shutdown();

    std::memset(&m_context, 0, sizeof(SceneContext));
}

void SceneRuntime::update(SceneTickContext& ctx) {
    m_systems.update(ctx);
}

SceneContext& SceneRuntime::context() {
    DEV_ASSERT(m_context[0]);
    return *reinterpret_cast<SceneContext*>(&m_context);
}

}  // namespace cave
