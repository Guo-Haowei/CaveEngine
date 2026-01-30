#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/math/AABB.h"
#include "cave/core/math/Matrix.h"

// @TODO: move MeshAsset away from MeshAsset
#include "engine/private/assets/mesh_asset.h"

// clang-format off
namespace cave { struct GpuMesh; }
namespace cave { struct GpuMaterial; }
// clang-format on

namespace cave::render {

// clang-format off
enum class RenderableFlags : uint32_t {
    None        = 0,
    Visible     = BIT(0),
    Transparent = BIT(1),
    CastShadow  = BIT(2),
    Skinned     = BIT(3),
    Selected    = BIT(4),
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(RenderableFlags)

constexpr uint16_t kInvalidPayload = 0xFFFFu;

struct PayloadRef {
    enum Kind : uint16_t {
        None = 0,
        Mesh,
        Sprite,
        TileMap,
        Debug,
    };
    Kind kind{ None };
    uint16_t index{ kInvalidPayload };
};

struct RenderableHeader {
    ecs::Entity owner{};
    PayloadRef payload{};
    RenderableFlags flags{ RenderableFlags::None };

    math::Matrix4x4f world{};
    math::AABB world_bound{};

    bool HasFlag(RenderableFlags p_flag) const { return !!std::to_underlying(flags & p_flag); }
};

// clang-format off
enum RenderDirtyFlags : uint32_t {
    RENGER_DIRTY_FLAG_NONE      = 0,
    RENGER_DIRTY_FLAG_TRANSFORM = BIT(0),
    RENGER_DIRTY_FLAG_MESH      = BIT(1),
    RENGER_DIRTY_FLAG_MATERIAL  = BIT(2),
    RENGER_DIRTY_FLAG_SKELETON  = BIT(3),
    RENGER_DIRTY_FLAG_ALL       = ~0u,
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(RenderDirtyFlags);

struct MeshPayload {
    const GpuMesh* mesh{ nullptr };
    math::AABB local_bound{};

    ecs::Entity skeleton{};
    std::vector<MeshAsset::MeshSubset> subsets;
    std::vector<ecs::Entity> materials;
};

struct SpritePayload {
    ecs::Entity material{};
};

class RenderScene {
public:
    void Clear();
    void ClearDirtyLists();

    std::vector<RenderableHeader> m_renderables;
    std::vector<MeshPayload> m_meshes;
    std::vector<SpritePayload> m_sprites;
};

}  // namespace cave::render
