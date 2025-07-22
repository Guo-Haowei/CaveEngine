#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/ecs/entity.h"
#include "engine/math/aabb.h"
#include "engine/math/geomath.h"

namespace cave {

class Archive;
struct BvhAccel;
struct GpuMesh;

enum class VertexAttributeName : uint8_t {
    POSITION = 0,
    NORMAL,
    TEXCOORD_0,
    TEXCOORD_1,
    TANGENT,
    JOINTS_0,
    WEIGHTS_0,
    COLOR_0,
    COUNT,
};

struct MeshAsset : public IAsset {
    CAVE_ASSET(MeshAsset, AssetType::Mesh, 0)

    enum : uint32_t {
        NONE = BIT(0),
        RENDERABLE = BIT(1),
        DYNAMIC = BIT(3),
    };

    struct VertexAttribute {
        VertexAttributeName attribName;
        uint32_t offsetInByte{ 0 };
        uint32_t strideInByte{ 0 };

        uint32_t elementCount{ 0 };
    };

    uint32_t flags = RENDERABLE;
    std::vector<uint32_t> indices;
    std::vector<Vector3f> positions;
    std::vector<Vector3f> normals;
    std::vector<Vector3f> tangents;
    std::vector<Vector2f> texcoords_0;
    std::vector<Vector2f> texcoords_1;
    std::vector<Vector4i> joints_0;
    std::vector<Vector4f> weights_0;
    std::vector<Vector4f> color_0;

    struct MeshSubset {
        Guid material_id;
        uint32_t index_offset = 0;
        uint32_t index_count = 0;
        AABB local_bound;

        // Non-serialized
        Handle<MaterialAsset> material_handle;
    };
    std::vector<MeshSubset> subsets;

    // @TODO: make it an asset
    ecs::Entity armatureId;

    // Non-serialized
    mutable std::shared_ptr<GpuMesh> gpuResource;
    mutable std::shared_ptr<BvhAccel> bvh;
    AABB localBound;

    mutable std::vector<Vector3f> updatePositions;
    mutable std::vector<Vector3f> updateNormals;

    VertexAttribute attributes[std::to_underlying(VertexAttributeName::COUNT)];

    std::vector<Guid> GetDependencies() const override;

    Result<void> SaveToDisk(const AssetMetaData& p_meta) const override;

    Result<void> LoadFromDisk(const AssetMetaData& p_meta) override;

    void CreateRenderData();

    void Serialize(Archive& p_archive, uint32_t p_version);

    void OnDeserialized();
};

}  // namespace cave
