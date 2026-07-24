// =============================================================================
// File: cave/runtime/tile_map/TileSetAsset.h
// =============================================================================
#pragma once
#include "cave/core/math/Box.h"
#include "cave/core/reflection/Reflection.h"
#include "cave/runtime/assets/AssetHandle.h"

namespace cave {

// @TODO: move layer & mask to collision
enum class CollisionType {
    None,
    Solid,
    Trigger,
    Count,
};

DECLARE_ENUM_TRAITS(CollisionType, "None", "Solid", "Trigger");

// @TODO: refactor this to use with SpriteAnimationClip as well
struct TileFrame {
    CAVE_META(TileFrame)

    CAVE_PROP()
    uint32_t atlas_index = 0;

    CAVE_PROP()
    float duration = 0.1f;
};

struct TileDefinition {
    CAVE_META(TileDefinition)

    CAVE_PROP()
    uint32_t id = 0;

    CAVE_PROP()
    CollisionType collision = CollisionType::None;

    CAVE_PROP()
    math::Box2 collision_shape = { math::Vec2f::Zero, math::Vec2f::One };

    CAVE_PROP(editor = BitMask)
    uint32_t layer = 0;

    CAVE_PROP(editor = BitMask)
    uint32_t mask = 0;

    // @TODO: fix
    int terrain_id = -1;
    uint16_t terrain_mask = 0;

    CAVE_PROP()
    Vector<TileFrame> animation;
};

class TileSetAsset : public IAsset {
    CAVE_ASSET(TileSetAsset, AssetType::TileSet, 0)

    CAVE_META(TileSetAsset)

public:
    static constexpr float kDefaultCellSizePx = 8.f;

    uint32_t row() const { return m_row; }
    void row(uint32_t row);

    uint32_t col() const { return m_column; }
    void col(uint32_t col);

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    float tileScale() const { return m_tile_scale; }
    void tileScale(float scale);

    TileDefinition* findTileDefinition(uint32_t tile_id);
    const TileDefinition* findTileDefinition(uint32_t tile_id) const;
    TileDefinition& getOrCreateTile(uint32_t index);

    std::span<const TileDefinition> getTileDefinitions() const {
        return m_definitions;
    }

    Vector<TileDefinition>& getTileDefinitionsMut() {
        return m_definitions;
    }

    const Guid& imageGuid() const { return m_image_guid; }
    void setImageGuid(const Guid& guid);

    const Handle<ImageAsset>& handle() const { return m_image_handle; }

    std::span<const math::Box2> frames() const { return m_frames; }

    bool dirty() const { return m_dirty; }
    void setDirty(bool dirty) { m_dirty = dirty; }

    auto saveToDisk(const AssetMetaData& meta) const -> Result<void> override;
    auto loadFromDisk(const AssetMetaData& meta) -> Result<void> override;

    Vector<Guid> dependencies() const override;

private:
    void setHandle(Handle<ImageAsset>&& handle);
    void updateFrames();

    CAVE_PROP(editor = Asset)
    Guid m_image_guid;

    CAVE_PROP(editor = DragFloat, min = 0.01f, max = 100.0f)
    float m_tile_scale = 1.0f;

    CAVE_PROP(editor = InputInt, min = 1, max = 1000)
    uint32_t m_row = 1;

    CAVE_PROP(editor = InputInt, min = 1, max = 1000)
    uint32_t m_column = 1;

    CAVE_PROP()
    Vector<TileDefinition> m_definitions;

    /// Non serialized
    Vector<math::Box2> m_frames;  // frames are calculated
    Handle<ImageAsset> m_image_handle;
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    bool m_dirty;
};

}  // namespace cave
