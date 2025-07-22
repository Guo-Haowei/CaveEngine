#include "mesh_asset.h"

#include "engine/assets/material_asset.h"
#include "engine/core/io/archive.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/graphics_manager_interface.h"

namespace cave {

template<typename T>
static void InitVertexAttrib(MeshAsset::VertexAttribute& p_attrib, const std::vector<T>& p_buffer) {
    p_attrib.offsetInByte = 0;
    p_attrib.strideInByte = sizeof(p_buffer[0]);
    p_attrib.elementCount = static_cast<uint32_t>(p_buffer.size());
}

void MeshAsset::CreateRenderData() {
    // AABB
    localBound.MakeInvalid();
    for (MeshSubset& subset : subsets) {
        subset.local_bound.MakeInvalid();
        for (uint32_t i = 0; i < subset.index_count; ++i) {
            const Vector3f& point = positions[indices[i + subset.index_offset]];
            subset.local_bound.ExpandPoint(reinterpret_cast<const Vector3f&>(point));
        }
        subset.local_bound.MakeValid();
        localBound.UnionBox(subset.local_bound);
    }
    // Attributes
    for (int i = 0; i < std::to_underlying(VertexAttributeName::COUNT); ++i) {
        attributes[i].attribName = static_cast<VertexAttributeName>(i);
    }

    InitVertexAttrib(attributes[std::to_underlying(VertexAttributeName::POSITION)], positions);
    InitVertexAttrib(attributes[std::to_underlying(VertexAttributeName::NORMAL)], normals);
    InitVertexAttrib(attributes[std::to_underlying(VertexAttributeName::TEXCOORD_0)], texcoords_0);
    InitVertexAttrib(attributes[std::to_underlying(VertexAttributeName::TEXCOORD_1)], texcoords_1);
    InitVertexAttrib(attributes[std::to_underlying(VertexAttributeName::TANGENT)], tangents);
    InitVertexAttrib(attributes[std::to_underlying(VertexAttributeName::JOINTS_0)], joints_0);
    InitVertexAttrib(attributes[std::to_underlying(VertexAttributeName::WEIGHTS_0)], weights_0);
    InitVertexAttrib(attributes[std::to_underlying(VertexAttributeName::COLOR_0)], color_0);
    return;
}

void MeshAsset::SerializeBinary(Archive& p_archive, uint32_t) {
    size_t subset_count = subsets.size();
    p_archive.ArchiveValue(subset_count);
    subsets.resize(subset_count);
    for (size_t i = 0; i < subset_count; ++i) {
        auto& subset = subsets[i];
        p_archive.ArchiveValue(subset.material_id);
        p_archive.ArchiveValue(subset.index_offset);
        p_archive.ArchiveValue(subset.index_count);
        p_archive.ArchiveValue(subset.local_bound);
    }

    p_archive.ArchiveValue(flags);
    p_archive.ArchiveValue(indices);
    p_archive.ArchiveValue(positions);
    p_archive.ArchiveValue(normals);
    p_archive.ArchiveValue(tangents);
    p_archive.ArchiveValue(texcoords_0);
    p_archive.ArchiveValue(texcoords_1);
    p_archive.ArchiveValue(joints_0);
    p_archive.ArchiveValue(weights_0);
    p_archive.ArchiveValue(color_0);

    // @TODO: get rid of it
    p_archive.ArchiveValue(armatureId);
}

void MeshAsset::OnDeserialized() {
    CreateRenderData();

    for (auto& it : subsets) {
        if (!it.material_id.IsNull()) {
            auto handle = AssetRegistry::GetSingleton().FindByGuid<MaterialAsset>(it.material_id);
            if (handle.is_some()) {
                it.material_handle = handle.unwrap_unchecked();
            }
        }
    }

    IGraphicsManager::GetSingleton().RequestMesh(this);
}

std::vector<Guid> MeshAsset::GetDependencies() const {
    std::vector<Guid> dependencies;
    dependencies.reserve(subsets.size());
    for (const auto& subset : subsets) {
        dependencies.push_back(subset.material_id);
    }
    return dependencies;
}

Result<void> MeshAsset::SaveToDisk(const AssetMetaData& p_meta) const {
    unused(p_meta);
#if 0
    if (auto res = p_meta.SaveToDisk(this); !res) {
        return CAVE_ERROR(res.error());
    }

    Archive archive;
    auto sys_path = FileAccess::FixPath(FileAccess::ACCESS_RESOURCE, p_meta.import_path);
    if (auto res = archive.OpenWrite(sys_path); !res) {
        return CAVE_ERROR(res.error());
    }

    // bypass the const check
    auto asset = const_cast<MeshAsset&>(*this);
    asset.SerializeBinary(archive, VERSION);
#endif
    return Result<void>();
}

Result<void> MeshAsset::LoadFromDisk(const AssetMetaData& p_meta) {
    unused(p_meta);
    return Result<void>();
}

}  // namespace cave
