// =============================================================================
// File: cave/runtime/ecs/ComponentDefines.h
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

#define REGISTER_COMPONENT_SERIALIZED_LIST                                                   \
    REGISTER_COMPONENT(NameComponent, "World::NameComponent", 0)                             \
    REGISTER_COMPONENT(HierarchyComponent, "World::HierarchyComponent", 0)                   \
    REGISTER_COMPONENT(TransformComponent, "World::TransformComponent", 0)                   \
    REGISTER_COMPONENT(CameraComponent, "World::CameraComponent", 0)                         \
    REGISTER_COMPONENT(LightComponent, "World::LightComponent", 0)                           \
    REGISTER_COMPONENT(TransformAnimationComponent, "World::TransformAnimationComponent", 0) \
    REGISTER_COMPONENT(SkeletalAnimationComponent, "World::SkeletalAnimationComponent", 0)   \
    REGISTER_COMPONENT(SkeletonComponent, "World::SkeletonComponent", 0)                     \
    REGISTER_COMPONENT(SpriteAnimatorComponent, "World::SpriteAnimatorComponent", 0)         \
    REGISTER_COMPONENT(ColliderComponent, "World::ColliderComponent", 0)                     \
    REGISTER_COMPONENT(TriggerComponent, "World::TriggerComponent", 0)                       \
    REGISTER_COMPONENT(ContactComponent, "World::ContactComponent", 0)                       \
    REGISTER_COMPONENT(MotorComponent, "World::MotorComponent", 0)                           \
    REGISTER_COMPONENT(VelocityComponent, "World::VelocityComponent", 0)                     \
    REGISTER_COMPONENT(LuaScriptComponent, "World::LuaScriptComponent", 0)                   \
    REGISTER_COMPONENT(NativeScriptComponent, "World::NativeScriptComponent", 0)             \
    REGISTER_COMPONENT(PrefabInstanceComponent, "World::PrefabInstanceComponent", 0)         \
    REGISTER_COMPONENT(MeshRendererComponent, "World::MeshRendererComponent", 0)             \
    REGISTER_COMPONENT(MaterialComponent, "World::MaterialComponent", 0)                     \
    REGISTER_COMPONENT(FacingComponent, "World::FacingComponent", 0)                         \
    REGISTER_COMPONENT(SpriteRendererComponent, "World::SpriteRendererComponent", 0)         \
    REGISTER_COMPONENT(BackgroundComponent, "World::BackgroundComponent", 0)                 \
    REGISTER_COMPONENT(TileMapInstanceComponent, "World::TileMapInstanceComponent", 0)       \
    REGISTER_COMPONENT(UICanvasComponent, "UI::UICanvasComponent", 0)                        \
    REGISTER_COMPONENT(UIRectTransformComponent, "UI::UIRectTransformComponent", 0)          \
    REGISTER_COMPONENT(UIImageComponent, "UI::UIImageComponent", 0)                          \
    REGISTER_COMPONENT(UITextComponent, "UI::UITextComponent", 0)                            \
    REGISTER_COMPONENT(UIButtonComponent, "UI::UIButtonComponent", 0)

#define REGISTER_COMPONENT_LIST                                                \
    REGISTER_COMPONENT_SERIALIZED_LIST                                         \
    REGISTER_COMPONENT(PrefabChildComponent, "World::PrefabChildComponent", 0) \
    REGISTER_COMPONENT(PendingDestroyComponent, "World::PendingDestroyComponent", 0)

enum BuiltinComponentId : ComponentId {
#define REGISTER_COMPONENT(TYPE, ...) TYPE##_Id,
    REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT
        Count,
};

}  // namespace cave
