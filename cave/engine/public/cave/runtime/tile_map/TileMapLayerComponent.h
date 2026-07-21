// =============================================================================
// File: cave/runtime/tile_map/TileMapLayerComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Box.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/tile_map/TileData.h"

namespace cave {

class TileMapLayer;
struct FieldChange;
struct GpuMesh;

class TileMapLayerComponent {
    CAVE_COMPONENT(TileMapLayerComponent)

public:
    const Handle<TileSetAsset>& handle() const { return m_tile_set_handle; }

    const Guid& tileSetGuid() const { return m_tile_set; }
    void setTileSetGuid(const Guid& guid);

    ChunkedTileData& chunks() { return m_chunks; }
    const ChunkedTileData& chunks() const { return m_chunks; }

    int zIndex() const { return m_z_index; }
    void setZIndex(int value) { m_z_index = value; }

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
};

class TileMapInstanceComponent {
    CAVE_COMPONENT(TileMapInstanceComponent)

public:
    struct TileCache {
        int16_t x, y;
        uint32_t tile_id;

        mutable float elapsed;
    };

    struct LayerCache {
        bool visible = true;
        int z_index = 0;

        Handle<TileSetAsset> tile_set;
        Handle<ImageAsset> image;

        Vector<TileCache> tiles;
    };

    // @TODO: remove this
    void createRenderData();

    const auto& tileMapHandle() const { return m_handle; }

    const Guid& tileMapGuid() const { return m_tile_map_guid; }

    std::span<const LayerCache> layers() const { return m_layers; }

    void onDeserialized();

private:
    void refreshTileMapHandle();
    void onTileMapGuidChanged(const FieldChange& change);

    bool updateLayer(const TileMapLayer& layer, LayerCache& cache);

    CAVE_PROP(editor = Asset, on_change = onTileMapGuidChanged)
    Guid m_tile_map_guid;

    // Non serialize
    Handle<TileMapAsset> m_handle;
    Vector<LayerCache> m_layers;
    uint32_t m_revision{ 0 };
};

}  // namespace cave
