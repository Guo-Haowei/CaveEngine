#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace cave {

void NativeScript::bind(SceneContext& ctx, ecs::Entity entity, const VariantMap& params) {
    m_entity = entity;
    m_params = params;
    m_context = &ctx;
}

void NativeScript::unbind() {
    m_entity = ecs::Entity::null();
    m_params.clear();
}

void* NativeScript::system(SceneSystemId system_id) {
    DEV_ASSERT(m_context);
    return m_context->runtime.systems().get(system_id);
}

}  // namespace cave