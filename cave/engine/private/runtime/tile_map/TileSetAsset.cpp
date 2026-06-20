#include "cave/runtime/tile_map/TileSetAsset.h"

#include "engine/private/core/io/file_access.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/serialization/yaml_include.h"

namespace cave {

void TileSetAsset::row(uint32_t row) {
    if (row == 0) return;
    if (row == row_) return;
    row_ = row;
    updateFrames();
}

void TileSetAsset::col(uint32_t col) {
    if (col == 0) return;
    if (col == column_) return;
    column_ = col;
    updateFrames();
}

void TileSetAsset::tileScale(float scale) {
    scale = glm::max(scale, 0.1f);
    if (scale != tile_scale_) {
        tile_scale_ = scale;
        // @TODO: dirty
    }
}

bool TileSetAsset::addBoxCollider(uint32_t tile_id) {
    if (tile_id < static_cast<uint32_t>(frames_.size())) {
        colliders_[tile_id] = Shape::MakeBox(math::Vec2f(0.5f));
        return true;
    }
    return false;
}

Option<Shape> TileSetAsset::getCollider(uint32_t tile_id) const {
    if (auto it = colliders_.find(tile_id); it != colliders_.end()) {
        return Some(it->second);
    }
    return None();
}

void TileSetAsset::setHandle(Handle<ImageAsset>&& handle) {
    image_handle_ = std::move(handle);
    const ImageAsset* image = image_handle_.Get();
    if (image) {
        Guid guid = image_handle_.GetGuid();
        if (guid != image_guid_) {
            LOG_INFO("TileSetAsset: GUID changed from {} to {}", image_guid_.ToString(), guid.ToString());
            image_guid_ = guid;
        }

        width_ = image->width;
        height_ = image->height;
    }
}

void TileSetAsset::setImage(const Guid& guid) {
    auto handle = AssetRegistry::singleton().FindByGuid<ImageAsset>(guid);
    if (handle.is_some()) {
        setHandle(std::move(handle.unwrap_unchecked()));
    }

    updateFrames();
}

std::vector<Guid> TileSetAsset::GetDependencies() const {
    return { image_guid_ };
}

void TileSetAsset::updateFrames() {
    DEV_ASSERT(row_ > 0 && column_ > 0);
    frames_.clear();
    frames_.reserve(row_ * column_);

    const float inv_w = 1.0f / column_;
    const float inv_h = 1.0f / row_;

    for (uint32_t y = 0; y < row_; ++y) {
        for (uint32_t x = 0; x < column_; ++x) {
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

            frames_.push_back(math::Box2({ u0, v0 }, { u1, v1 }));
        }
    }

    dirty_ = true;
}

auto TileSetAsset::SaveToDisk(const AssetMetaData& meta) const -> Result<void> {
    // meta
    auto res = meta.SaveToDisk(this);
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
    return SaveYaml(meta.import_path, yaml);
}

auto TileSetAsset::LoadFromDisk(const AssetMetaData& meta) -> Result<void> {
    YAML::Node root;

    if (auto res = LoadYaml(meta.import_path, root); !res) {
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
    auto handle = AssetRegistry::singleton().FindByGuid<ImageAsset>(image_guid_);
    if (handle.is_some()) {
        setHandle(std::move(handle.unwrap_unchecked()));
    }
    updateFrames();

    return Result<void>();
}

}  // namespace cave
