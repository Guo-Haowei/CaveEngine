// =============================================================================
// File: cave/runtime/tile_map/TileSetAsset.h
// =============================================================================
#pragma once
#include "cave/core/math/Box.h"
#include "cave/core/reflection/Reflection.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/components/ColliderComponent.h"

// @TODO: move layer & mask to shape, and rename it collision shape

namespace cave {

class TileSetAsset : public IAsset {
    CAVE_ASSET(TileSetAsset, AssetType::TileSet, 0)

    CAVE_META(TileSetAsset)

private:
    CAVE_PROP(editor = Asset)
    Guid m_image_guid;

    CAVE_PROP(editor = DragFloat, min = 0.01f, max = 100.0f)
    float m_tile_scale = 1.0f;

    CAVE_PROP()
    uint32_t m_width = 0;

    CAVE_PROP()
    uint32_t m_height = 0;

    CAVE_PROP(editor = InputInt, min = 1, max = 1000)
    uint32_t m_row = 1;

    CAVE_PROP(editor = InputInt, min = 1, max = 1000)
    uint32_t m_column = 1;

    CAVE_PROP()
    std::map<uint32_t, Shape> m_colliders;

    /// Non serialized
    std::vector<math::Box2> m_frames;  // frames are calculated
    Handle<ImageAsset> m_image_handle;
    bool m_dirty;

public:
    uint32_t row() const { return m_row; }
    void row(uint32_t row);

    uint32_t col() const { return m_column; }
    void col(uint32_t col);

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    float tileScale() const { return m_tile_scale; }
    void tileScale(float scale);

    bool addBoxCollider(uint32_t tile_id);
    Option<Shape> getCollider(uint32_t tile_id) const;

    const Guid& imageGuid() const { return m_image_guid; }
    void setImageGuid(const Guid& guid);

    const Handle<ImageAsset>& handle() const { return m_image_handle; }

    const auto& frames() const { return m_frames; }

    bool dirty() const { return m_dirty; }
    void setDirty(bool dirty) { m_dirty = dirty; }

    auto saveToDisk(const AssetMetaData& meta) const -> Result<void> override;
    auto loadFromDisk(const AssetMetaData& meta) -> Result<void> override;

    Vector<Guid> dependencies() const override;

private:
    void setHandle(Handle<ImageAsset>&& handle);
    void updateFrames();
};

}  // namespace cave
