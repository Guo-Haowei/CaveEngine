// =============================================================================
// File: engine/public/cave/runtime/ecs/ComponentDefines.h
// =============================================================================
#pragma once
#include <type_traits>
#include "cave/core/reflection/Reflection.h"

#define CAVE_COMPONENT(TYPE)                                      \
    CAVE_META(TYPE)                                               \
    static constexpr ::cave::ComponentId kId = ::cave::TYPE##_Id; \
    ::cave::ComponentId GetId() const { return kId; }

namespace cave {

using ComponentId = uint16_t;

template<typename T>
concept ComponentType = requires(T& t) {
    { T::kId } -> std::convertible_to<::cave::ComponentId>;
    { t.GetId() } -> std::same_as<::cave::ComponentId>;
};

#define REGISTER_COMPONENT_SERIALIZED_LIST                                                 \
    REGISTER_COMPONENT(NameComponent, "World::NameComponent", 0)                           \
    REGISTER_COMPONENT(HierarchyComponent, "World::HierarchyComponent", 0)                 \
    REGISTER_COMPONENT(TransformComponent, "World::TransformComponent", 0)                 \
    REGISTER_COMPONENT(CameraComponent, "World::CameraComponent", 0)                       \
    REGISTER_COMPONENT(LightComponent, "World::LightComponent", 0)                         \
    REGISTER_COMPONENT(SkeletalAnimationComponent, "World::SkeletalAnimationComponent", 0) \
    REGISTER_COMPONENT(SkeletonComponent, "World::SkeletonComponent", 0)                   \
    REGISTER_COMPONENT(SpriteAnimatorComponent, "World::SpriteAnimatorComponent", 0)       \
    REGISTER_COMPONENT(ColliderComponent, "World::ColliderComponent", 0)                   \
    REGISTER_COMPONENT(VelocityComponent, "World::VelocityComponent", 0)                   \
    REGISTER_COMPONENT(LuaScriptComponent, "World::LuaScriptComponent", 0)                 \
    REGISTER_COMPONENT(PrefabInstanceComponent, "World::PrefabInstanceComponent", 0)       \
    REGISTER_COMPONENT(MeshRendererComponent, "World::MeshRendererComponent", 0)           \
    REGISTER_COMPONENT(MaterialComponent, "World::MaterialComponent", 0)                   \
    REGISTER_COMPONENT(SpriteRendererComponent, "World::SpriteRendererComponent", 0)       \
    REGISTER_COMPONENT(TileMapRendererComponent, "World::TileMapRendererComponent", 0)

// @TODO: use meta table for all components
#define REGISTER_COMPONENT_LIST        \
    REGISTER_COMPONENT_SERIALIZED_LIST \
    REGISTER_COMPONENT(NoSaveTag, "World::NoSaveTag", 0)

enum BuiltinComponentId : ComponentId {
#define REGISTER_COMPONENT(TYPE, ...) TYPE##_Id,
    REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT
        _Count,
};

}  // namespace cave
