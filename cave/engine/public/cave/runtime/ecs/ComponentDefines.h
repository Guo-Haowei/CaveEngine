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

using ComponentId = StringId;

template<typename T>
concept ComponentType = requires(T& t) {
    { T::kId } -> std::convertible_to<::cave::ComponentId>;
    { t.GetId() } -> std::same_as<::cave::ComponentId>;
};

#define REGISTER_COMPONENT_SERIALIZED_LIST                                                             \
    REGISTER_COMPONENT(NameComponent, CAVE_SID("World::NameComponent"), 0)                             \
    REGISTER_COMPONENT(HierarchyComponent, CAVE_SID("World::HierarchyComponent"), 0)                   \
    REGISTER_COMPONENT(TransformComponent, CAVE_SID("World::TransformComponent"), 0)                   \
    REGISTER_COMPONENT(CameraComponent, CAVE_SID("World::CameraComponent"), 0)                         \
    REGISTER_COMPONENT(LightComponent, CAVE_SID("World::LightComponent"), 0)                           \
    REGISTER_COMPONENT(TransformAnimationComponent, CAVE_SID("World::TransformAnimationComponent"), 0) \
    REGISTER_COMPONENT(SkeletalAnimationComponent, CAVE_SID("World::SkeletalAnimationComponent"), 0)   \
    REGISTER_COMPONENT(SkeletonComponent, CAVE_SID("World::SkeletonComponent"), 0)                     \
    REGISTER_COMPONENT(SpriteAnimatorComponent, CAVE_SID("World::SpriteAnimatorComponent"), 0)         \
    REGISTER_COMPONENT(ColliderComponent, CAVE_SID("World::ColliderComponent"), 0)                     \
    REGISTER_COMPONENT(TriggerComponent, CAVE_SID("World::TriggerComponent"), 0)                       \
    REGISTER_COMPONENT(ContactComponent, CAVE_SID("World::ContactComponent"), 0)                       \
    REGISTER_COMPONENT(MotorComponent, CAVE_SID("World::MotorComponent"), 0)                           \
    REGISTER_COMPONENT(VelocityComponent, CAVE_SID("World::VelocityComponent"), 0)                     \
    REGISTER_COMPONENT(LuaScriptComponent, CAVE_SID("World::LuaScriptComponent"), 0)                   \
    REGISTER_COMPONENT(NativeScriptComponent, CAVE_SID("World::NativeScriptComponent"), 0)             \
    REGISTER_COMPONENT(PrefabInstanceComponent, CAVE_SID("World::PrefabInstanceComponent"), 0)         \
    REGISTER_COMPONENT(MeshRendererComponent, CAVE_SID("World::MeshRendererComponent"), 0)             \
    REGISTER_COMPONENT(MaterialComponent, CAVE_SID("World::MaterialComponent"), 0)                     \
    REGISTER_COMPONENT(FacingComponent, CAVE_SID("World::FacingComponent"), 0)                         \
    REGISTER_COMPONENT(SpriteRendererComponent, CAVE_SID("World::SpriteRendererComponent"), 0)         \
    REGISTER_COMPONENT(BackgroundComponent, CAVE_SID("World::BackgroundComponent"), 0)                 \
    REGISTER_COMPONENT(TileMapLayerComponent, CAVE_SID("World::TileMapLayerComponent"), 0)             \
    REGISTER_COMPONENT(UICanvasComponent, CAVE_SID("UI::UICanvasComponent"), 0)                        \
    REGISTER_COMPONENT(UIRectTransformComponent, CAVE_SID("UI::UIRectTransformComponent"), 0)          \
    REGISTER_COMPONENT(UIImageComponent, CAVE_SID("UI::UIImageComponent"), 0)                          \
    REGISTER_COMPONENT(UITextComponent, CAVE_SID("UI::UITextComponent"), 0)                            \
    REGISTER_COMPONENT(UIButtonComponent, CAVE_SID("UI::UIButtonComponent"), 0)

#define REGISTER_COMPONENT_LIST                                                          \
    REGISTER_COMPONENT_SERIALIZED_LIST                                                   \
    REGISTER_COMPONENT(PrefabChildComponent, CAVE_SID("World::PrefabChildComponent"), 0) \
    REGISTER_COMPONENT(PendingDestroyComponent, CAVE_SID("World::PendingDestroyComponent"), 0)

#define REGISTER_COMPONENT(TYPE, SOURCE, ...) \
    constexpr StringId TYPE##_Id = SOURCE;    \
    constexpr uint64_t TYPE##_hash = (TYPE##_Id).hash();
REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT

}  // namespace cave
