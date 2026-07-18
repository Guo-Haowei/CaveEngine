// =============================================================================
// File: cave/runtime/tile_map/TileMapInstanceComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Box.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class TileMapLayer;
struct FieldChange;
struct GpuMesh;

class TileMapInstanceComponent {
    CAVE_COMPONENT(TileMapInstanceComponent)

private:
    CAVE_PROP(editor = Asset, on_change = onTileMapGuidChanged)
    Guid m_tile_map_guid;

    struct Cache {
        Handle<ImageAsset> image;
        Handle<TileSetAsset> tile_set_handle;
        mutable Ref<GpuMesh> mesh;
    };

    // Non serialize
    Handle<TileMapAsset> m_handle;
    Vector<Cache> m_layers;
    uint32_t m_revision{ 0 };

    void refreshTileMapHandle();
    void onTileMapGuidChanged(const FieldChange& change);

    bool updateLayer(const TileMapLayer& layer, Cache& cache);

public:
    // @TODO: better way to create data
    void createRenderData();

    const auto& tileMapHandle() const { return m_handle; }

    const Guid& tileMapGuid() const { return m_tile_map_guid; }

    std::span<const Cache> layers() const { return m_layers; }

    void onDeserialized();
};

}  // namespace cave
