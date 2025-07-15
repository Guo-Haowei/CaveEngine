#pragma once

namespace cave {

enum class ToolType {
    None,
    Edit,
    TileMap,
    SpriteAnimation,
    Count,
};

enum class ToolCameraPolicy {
    Any,
    Only2D,
};

enum class GizmoAction : uint8_t {
    Translate,
    Rotate,
    Scale,
};

// clang-format off
//------------ Enum,                Name,           Separator
#define ENTITY_TYPE_LIST                                   \
    ENTITY_TYPE(INFINITE_LIGHT,		InfiniteLight,  false) \
    ENTITY_TYPE(POINT_LIGHT,		PointLight,     false) \
    ENTITY_TYPE(ENVIRONMENT,        Environment,    false) \
    ENTITY_TYPE(AREA_LIGHT,			AreaLight,      false ) \
    ENTITY_TYPE(VOXEL_GI,			VoxelGi,      true ) \
    ENTITY_TYPE(TRANSFORM,			Transform,      false) \
    ENTITY_TYPE(PLANE,				Plane,          false) \
    ENTITY_TYPE(CUBE,				Cube,           false) \
    ENTITY_TYPE(SPHERE,             Sphere,         false) \
    ENTITY_TYPE(CYLINDER,           Cylinder,       false) \
    ENTITY_TYPE(TORUS,              Torus,          true ) \
    ENTITY_TYPE(PARTICLE_EMITTER,   Emitter,        false) \
    ENTITY_TYPE(FORCE_FIELD,        ForceField,     false)
// clang-format on

enum class EntityType : uint8_t {
#define ENTITY_TYPE(ENUM, ...) ENUM,
    ENTITY_TYPE_LIST
#undef ENTITY_TYPE
        COUNT,
};

enum class ComponentType : uint8_t {
    Script,
    TileMap,
    Count,
};

}  // namespace cave
