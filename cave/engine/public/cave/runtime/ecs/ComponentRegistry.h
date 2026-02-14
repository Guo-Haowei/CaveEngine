// =============================================================================
// File: engine/public/cave/runtime/ecs/ComponentRegistry.h
// =============================================================================
#pragma once
#include <span>
#include <string_view>
#include <vector>
#include "cave/core/reflection/Reflection.h"

// @TODO: move the following list to built-in components

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
#define REGISTER_COMPONENT_LIST                                                \
    REGISTER_COMPONENT_SERIALIZED_LIST                                         \
    REGISTER_COMPONENT(RigidBodyComponent, "World::RigidBodyComponent", 0)     \
    REGISTER_COMPONENT(VoxelGiComponent, "World::VoxelGiComponent", 0)         \
    REGISTER_COMPONENT(EnvironmentComponent, "World::EnvironmentComponent", 0) \
    REGISTER_COMPONENT(NoSaveTag, "World::NoSaveTag", 0)

namespace cave {

enum BuildInComponentId : uint16_t {
#define REGISTER_COMPONENT(TYPE, ...) TYPE##_Id,
    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT
        _Count,
};

using ComponentId = uint16_t;

using PropertyId = std::string_view;

struct ComponentMeta {
    ComponentId id;
    const char* name;
    uint32_t size;
    uint32_t align;
    uint64_t version;

    std::span<const FieldMetaBase*> props;

    const FieldMetaBase* Find(PropertyId p_id) const;
};

class ComponentRegistry {
public:
    void Register(const ComponentMeta& p_meta);
    const ComponentMeta* TryGet(ComponentId p_id) const;

private:
    std::vector<ComponentMeta> m_table;
    std::vector<uint8_t> m_present;
};

}  // namespace cave
