// =============================================================================
// File: cave/runtime/tile_map/TileSetAsset.h
// =============================================================================
#pragma once
#include "cave/core/math/Box.h"
#include "cave/core/reflection/Reflection.h"

#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/assets/IAsset.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"

// @TODO: move layer & mask to shape, and rename it collision shape

namespace cave {

class TileSetAsset : public IAsset {
    CAVE_ASSET(TileSetAsset, AssetType::TileSet, 0)

    CAVE_META(TileSetAsset)

private:
    CAVE_PROP(editor = Asset)
    Guid image_guid_;

    CAVE_PROP(editor = DragFloat, min = 0.01f, max = 100.0f)
    float tile_scale_ = 1.0f;

    CAVE_PROP()
    uint32_t width_ = 0;

    CAVE_PROP()
    uint32_t height_ = 0;

    CAVE_PROP(editor = InputInt, min = 1, max = 1000)
    uint32_t row_ = 1;

    CAVE_PROP(editor = InputInt, min = 1, max = 1000)
    uint32_t column_ = 1;

    CAVE_PROP()
    std::map<uint32_t, Shape> colliders_;

    /// Non serialized
    std::vector<math::Box2> frames_;  // frames are calculated
    Handle<ImageAsset> image_handle_;
    bool dirty_;

public:
    uint32_t row() const { return row_; }
    void row(uint32_t row);

    uint32_t col() const { return column_; }
    void col(uint32_t col);

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }

    float tileScale() const { return tile_scale_; }
    void tileScale(float scale);

    bool addBoxCollider(uint32_t tile_id);
    Option<Shape> getCollider(uint32_t tile_id) const;

    void setImage(const Guid& guid);
    const Handle<ImageAsset>& handle() const { return image_handle_; }

    const Guid& GetImageGuid() const { return image_guid_; }

    const auto& GetFrames() const { return frames_; }

    bool dirty() const { return dirty_; }
    void dirty(bool dirty) { dirty_ = dirty; }

    auto saveToDisk(const AssetMetaData& meta) const -> Result<void> override;
    auto loadFromDisk(const AssetMetaData& meta) -> Result<void> override;

    Vector<Guid> dependencies() const override;

private:
    void setHandle(Handle<ImageAsset>&& handle);
    void updateFrames();
};

}  // namespace cave
