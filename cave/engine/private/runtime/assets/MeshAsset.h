#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/math/AABB.h"

#include "engine/private/assets/asset_handle.h"
#include "engine/private/core/math/geomath.h"

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

class MeshAsset : public IAsset {
    CAVE_ASSET(MeshAsset, AssetType::Mesh, 0)

public:
    struct VertexAttribute {
        VertexAttributeName attribName;
        uint32_t offsetInByte{ 0 };
        uint32_t strideInByte{ 0 };

        uint32_t elementCount{ 0 };
    };

    uint32_t flags = 0;
    std::vector<uint32_t> indices;
    std::vector<math::Vector3f> positions;
    std::vector<math::Vector3f> normals;
    std::vector<math::Vector3f> tangents;
    std::vector<math::Vector2f> texcoords_0;
    std::vector<math::Vector2f> texcoords_1;
    std::vector<math::Vector4i> joints_0;
    std::vector<math::Vector4f> weights_0;
    std::vector<math::Vector4f> color_0;

    struct MeshSubset {
        uint32_t index_offset = 0;
        uint32_t index_count = 0;
        math::AABB local_bound;
    };
    std::vector<MeshSubset> subsets;

    // Non-serialized
    mutable std::shared_ptr<GpuMesh> gpuResource;
    mutable std::shared_ptr<BvhAccel> bvh;
    math::AABB localBound;

    mutable std::vector<math::Vector3f> updatePositions;
    mutable std::vector<math::Vector3f> updateNormals;

    VertexAttribute attributes[std::to_underlying(VertexAttributeName::COUNT)];

    std::vector<Guid> GetDependencies() const override;

    Result<void> SaveToDisk(const AssetMetaData& p_meta) const override;

    Result<void> LoadFromDisk(const AssetMetaData& p_meta) override;

    void CreateRenderData();

    void SerializeBinary(Archive& p_archive, uint32_t p_version);

    void OnDeserialized();
};

}  // namespace cave
