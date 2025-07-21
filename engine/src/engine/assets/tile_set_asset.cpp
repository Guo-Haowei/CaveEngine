#include "tile_set_asset.h"

#include "engine/assets/image_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/runtime/asset_registry.h"
#include "engine/serialization/yaml_include.h"

namespace cave {

void TileSetAsset::SetRow(uint32_t p_row) {
    if (p_row == 0) return;
    if (p_row == m_row) return;
    m_row = p_row;
    UpdateFrames();
}

void TileSetAsset::SetCol(uint32_t p_col) {
    if (p_col == 0) return;
    if (p_col == m_column) return;
    m_column = p_col;
    UpdateFrames();
}

void TileSetAsset::SetScale(float p_scale) {
    p_scale = glm::max(p_scale, 0.1f);
    if (p_scale != m_tile_scale) {
        m_tile_scale = p_scale;
        // @TODO: dirty
    }
}

void TileSetAsset::SetHandle(Handle<ImageAsset>&& p_handle) {
    m_image_handle = std::move(p_handle);
    const ImageAsset* image = m_image_handle.Get();
    if (image) {
        Guid guid = m_image_handle.GetGuid();
        if (guid != m_image_guid) {
            LOG("TileSetAsset: GUID changed from {} to {}", m_image_guid.ToString(), guid.ToString());
            m_image_guid = guid;
        }

        m_width = image->width;
        m_height = image->height;
    }
}

void TileSetAsset::SetImage(const Guid& p_guid) {
    auto handle = AssetRegistry::GetSingleton().FindByGuid<ImageAsset>(p_guid);
    if (handle.is_some()) {
        SetHandle(std::move(handle.unwrap_unchecked()));
    }

    UpdateFrames();
}

std::vector<Guid> TileSetAsset::GetDependencies() const {
    return { m_image_guid };
}

void TileSetAsset::UpdateFrames() {
    DEV_ASSERT(m_row > 0 && m_column > 0);
    m_frames.clear();
    m_frames.reserve(m_row * m_column);

    const float inv_w = 1.0f / m_column;
    const float inv_h = 1.0f / m_row;

    for (uint32_t y = 0; y < m_row; ++y) {
        for (uint32_t x = 0; x < m_column; ++x) {
            // flip y here because in ndc it's up is +y, down -y
            // but in uv space, up is 0, down is 1
#if 1
            const float u0 = (x + 0) * inv_w;
            const float v0 = (y + 0) * inv_h;
            const float u1 = (x + 1) * inv_w;
            const float v1 = (y + 1) * inv_h;
#else
            const float u0 = (x + 0) * inv_w;
            const float v0 = (y + 1) * inv_h;
            const float u1 = (x + 1) * inv_w;
            const float v1 = (y + 0) * inv_h;
#endif

            m_frames.push_back(Rect({ u0, v0 }, { u1, v1 }));
        }
    }

    m_dirty = true;
}

auto TileSetAsset::SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> {
    // meta
    auto res = p_meta.SaveToDisk(this);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    YamlSerializer yaml;
    yaml.BeginMap(false)
        .Key("version")
        .Write(VERSION)
        .Key("content")
        .Write(*this)
        .EndMap();
    return SaveYaml(p_meta.path, yaml);
}

auto TileSetAsset::LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> {
    YAML::Node root;

    if (auto res = LoadYaml(p_meta.path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer deserializer;
    deserializer.Initialize(root);

    const int version = deserializer.GetVersion();

    if (deserializer.TryEnterKey("content")) {
        switch (version) {
            case 1:
                [[fallthrough]];
            default:
                deserializer.Read(*this);
                break;
        }

        deserializer.LeaveKey();
    }

    // @TODO: post load?
    auto handle = AssetRegistry::GetSingleton().FindByGuid<ImageAsset>(m_image_guid);
    if (handle.is_some()) {
        SetHandle(std::move(handle.unwrap_unchecked()));
    }
    UpdateFrames();

    return Result<void>();
}

}  // namespace cave
