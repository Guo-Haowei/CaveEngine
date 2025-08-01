#pragma once

namespace cave {

enum class GizmoAction : uint8_t {
    Translate,
    Rotate,
    Scale,
};

// clang-format off
//              Name,           Separator
#define ENTITY_TYPE_LIST               \
    ENTITY_TYPE(InfiniteLight,  false) \
    ENTITY_TYPE(PointLight,     false) \
    ENTITY_TYPE(Environment,    false) \
    ENTITY_TYPE(AreaLight,      false) \
    ENTITY_TYPE(VoxelGi,        true ) \
    ENTITY_TYPE(Transform,      false) \
    ENTITY_TYPE(Plane,          false) \
    ENTITY_TYPE(Cube,           false) \
    ENTITY_TYPE(Sphere,         false) \
    ENTITY_TYPE(Cylinder,       false) \
    ENTITY_TYPE(Cone,           false) \
    ENTITY_TYPE(Torus,          true )
// clang-format on

// ENTITY_TYPE(PARTICLE_EMITTER,   Emitter,        false)
// ENTITY_TYPE(FORCE_FIELD,        ForceField,     false)

enum class EntityType : uint8_t {
#define ENTITY_TYPE(NAME, ...) NAME,
    ENTITY_TYPE_LIST
#undef ENTITY_TYPE
        Count,
};

#define COMPONENT_LIST              \
    COMPONENT_DECL(LuaScript)       \
    COMPONENT_DECL(Animator)        \
    COMPONENT_DECL(Collider)        \
    COMPONENT_DECL(Velocity)        \
    COMPONENT_DECL(MeshRenderer)    \
    COMPONENT_DECL(SpriteRenderer)  \
    COMPONENT_DECL(TileMapRenderer) \
    COMPONENT_DECL(PrefabInstance)

enum class ComponentName : uint8_t {
#define COMPONENT_DECL(NAME) NAME,
    COMPONENT_LIST
#undef COMPONENT_DECL
};

}  // namespace cave
