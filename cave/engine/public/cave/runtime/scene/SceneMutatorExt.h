// =============================================================================
// File: public/cave/runtime/scene/SceneMutatorExt.h
// =============================================================================
#pragma once
#include <string_view>
#include "cave/runtime/ecs/Entity.h"

namespace cave {

class SceneCommandBuffer;

ecs::Entity CreateNameObject(SceneCommandBuffer& p_cb, std::string_view p_name);

ecs::Entity CreateTransformObject(SceneCommandBuffer& p_cb, std::string_view p_name);

ecs::Entity CreateMeshObject(SceneCommandBuffer& p_cb, std::string_view p_name);

}  // namespace cave
