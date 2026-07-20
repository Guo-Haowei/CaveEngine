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

    // Non serialize
    struct AnimationFrame {
        float duration;
        uint32_t offset;
        uint32_t count;
    };

    struct LayerCache {
        bool visible = true;
        int z_index = 0;

        Handle<TileSetAsset> tile_set_handle;
        Handle<ImageAsset> image;

        Ref<GpuMesh> mesh;
        Ref<GpuMesh> animated_mesh;

        Vector<AnimationFrame> animation;

        bool isStatic() const { return animation.empty(); }
    };

    Handle<TileMapAsset> m_handle;
    Vector<LayerCache> m_layers;
    uint32_t m_revision{ 0 };

    void refreshTileMapHandle();
    void onTileMapGuidChanged(const FieldChange& change);

    bool updateLayer(const TileMapLayer& layer, LayerCache& cache);

public:
    // @TODO: better way to create data
    void createRenderData();

    const auto& tileMapHandle() const { return m_handle; }

    const Guid& tileMapGuid() const { return m_tile_map_guid; }

    std::span<const LayerCache> layers() const { return m_layers; }

    void onDeserialized();
};

}  // namespace cave
