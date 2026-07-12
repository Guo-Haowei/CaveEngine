#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/script/native/NativeScript.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

void NativeScript::bind(SceneRuntime* runtime, ecs::Entity entity, const VariantMap& params) {
    m_entity = entity;
    m_params = params;
    m_runtime = runtime;
}

void NativeScript::unbind() {
    m_entity = ecs::Entity::null();
    m_params.clear();
    m_runtime = nullptr;
}

}  // namespace cave