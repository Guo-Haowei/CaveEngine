#include "cave/runtime/tile_map/TileSetAsset.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/serialization/YamlInclude.h"

namespace cave {

using namespace ::cave::math;

void TileSetAsset::row(uint32_t row) {
    if (row == 0) return;
    if (row == m_row) return;
    m_row = row;
    updateFrames();
}

void TileSetAsset::col(uint32_t col) {
    if (col == 0) return;
    if (col == m_column) return;
    m_column = col;
    updateFrames();
}

void TileSetAsset::tileScale(float scale) {
    scale = glm::max(scale, 0.1f);
    if (scale != m_tile_scale) {
        m_tile_scale = scale;
        // @TODO: dirty
    }
}

Option<Shape> TileSetAsset::getCollider(uint32_t tile_id) const {
    unused(tile_id);
    DEV_ASSERT(0);
    return None();
}

void TileSetAsset::setHandle(Handle<ImageAsset>&& handle) {
    m_image_handle = std::move(handle);
    const ImageAsset* image = m_image_handle.get();
    if (image) {
        Guid guid = m_image_handle.guid();
        if (guid != m_image_guid) {
            LOG_INFO("TileSetAsset: GUID changed from {} to {}", m_image_guid.toString(), guid.toString());
            m_image_guid = guid;
        }

        m_width = image->width;
        m_height = image->height;
    }
}

void TileSetAsset::setImageGuid(const Guid& guid) {
    auto handle = AssetRegistry::singleton().findByGuid<ImageAsset>(guid);
    if (handle.is_some()) {
        setHandle(std::move(handle.unwrap_unchecked()));
    }

    updateFrames();
}

Vector<Guid> TileSetAsset::dependencies() const {
    return { m_image_guid };
}

void TileSetAsset::updateFrames() {
    DEV_ASSERT(m_row > 0 && m_column > 0);
    m_frames.clear();
    m_frames.reserve(m_row * m_column);

    const float inv_w = 1.0f / m_column;
    const float inv_h = 1.0f / m_row;

    for (uint32_t y = 0; y < m_row; ++y) {
        for (uint32_t x = 0; x < m_column; ++x) {
            const float u0 = static_cast<float>(x) * inv_w;
            const float v0 = static_cast<float>(y) * inv_h;
            const float u1 = static_cast<float>(x + 1) * inv_w;
            const float v1 = static_cast<float>(y + 1) * inv_h;
            m_frames.emplace_back(Box2({ u0, v0 }, { u1, v1 }));
        }
    }

    m_dirty = true;
}

auto TileSetAsset::saveToDisk(const AssetMetaData& meta) const -> Result<void> {
    // meta
    auto res = meta.saveToDisk(this);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    YamlSerializer yaml;
    yaml.beginMap(false)
        .beginKey("version")
        .write(kVersion)
        .beginKey("content")
        .write(*this)
        .endMap();
    return SaveYaml(meta.import_path, yaml);
}

auto TileSetAsset::loadFromDisk(const AssetMetaData& meta) -> Result<void> {
    YAML::Node root;

    if (auto res = LoadYaml(meta.import_path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer deserializer;
    deserializer.initialize(root);

    const int version = deserializer.version();

    if (deserializer.tryEnterKey("content")) {
        switch (version) {
            case 1:
                [[fallthrough]];
            default:
                deserializer.read(*this);
                break;
        }

        deserializer.leaveKey();
    }

    // @TODO: post load?
    auto handle = AssetRegistry::singleton().findByGuid<ImageAsset>(m_image_guid);
    if (handle.is_some()) {
        setHandle(std::move(handle.unwrap_unchecked()));
    }
    updateFrames();

    return Result<void>();
}

}  // namespace cave
