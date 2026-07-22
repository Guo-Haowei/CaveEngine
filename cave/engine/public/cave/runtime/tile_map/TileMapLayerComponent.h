// =============================================================================
// File: cave/runtime/tile_map/TileMapLayerComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Box.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/tile_map/TileData.h"

namespace cave {

struct FieldChange;
struct GpuMesh;

class TileMapLayerComponent {
    CAVE_COMPONENT(TileMapLayerComponent)

public:
    struct TileCache {
        int16_t x, y;
        uint32_t tile_id;

        mutable float elapsed;
    };

    const Guid& tileSetGuid() const { return m_tile_set; }
    void setTileSetGuid(const Guid& guid);

    ChunkedTileData& chunks() { return m_chunks; }
    const ChunkedTileData& chunks() const { return m_chunks; }

    int zIndex() const { return m_z_index; }
    void setZIndex(int value) { m_z_index = value; }

    const Handle<TileSetAsset>& tileSetHandle() const { return m_tile_set_handle; }
    const Handle<ImageAsset>& imageHandle() const { return m_image_handle; }

    std::span<const TileCache> getTileCache() const { return m_tile_cache; }

    void updateTileCache();
    void onDeserialized();

private:
    void refreshTileSetHandle();
    void onTileSetGuidChanged(const FieldChange& change);

    CAVE_PROP(editor = Asset, on_change = onTileSetGuidChanged)
    Guid m_tile_set;

    CAVE_PROP(editor = InputInt)
    int m_z_index = 0;

    CAVE_PROP()
    ChunkedTileData m_chunks;

    // Non serialized
    Handle<TileSetAsset> m_tile_set_handle;
    Handle<ImageAsset> m_image_handle;

    Vector<TileCache> m_tile_cache;
};

}  // namespace cave
