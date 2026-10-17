#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/math/AABB.h"
#include "cave/runtime/assets/AssetHandle.h"

// @TODO: refactor
namespace cave::render {
struct GpuMesh;
class BvhAccel;
}  // namespace cave::render

namespace cave {

class Archive;

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
    Vector<uint32_t> indices;
    Vector<math::Vec3f> positions;
    Vector<math::Vec3f> normals;
    Vector<math::Vec3f> tangents;
    Vector<math::Vec2f> texcoords_0;
    Vector<math::Vec2f> texcoords_1;
    Vector<math::Vec4i> joints_0;
    Vector<math::Vec4f> weights_0;
    Vector<math::Vec4f> color_0;

    struct MeshSubset {
        uint32_t index_offset = 0;
        uint32_t index_count = 0;
        math::AABB local_bound;
    };
    Vector<MeshSubset> subsets;

    // Non-serialized
    mutable Ref<render::GpuMesh> gpuResource;
    mutable Ref<render::BvhAccel> bvh;
    math::AABB localBound;

    mutable Vector<math::Vec3f> updatePositions;
    mutable Vector<math::Vec3f> updateNormals;

    VertexAttribute attributes[std::to_underlying(VertexAttributeName::COUNT)];

    Vector<Guid> dependencies() const override;

    Result<void> saveToDisk(const AssetMetaData& p_meta) const override;

    Result<void> loadFromDisk(const AssetMetaData& p_meta) override;

    void CreateRenderData();

    void SerializeBinary(Archive& p_archive, uint32_t p_version);

    void onDeserialized();
};

}  // namespace cave
