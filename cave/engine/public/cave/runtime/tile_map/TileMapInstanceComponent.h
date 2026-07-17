// =============================================================================
// File: cave/runtime/tile_map/TileMapInstanceComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Box.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct FieldChange;
struct GpuMesh;

class TileMapInstanceComponent {
    CAVE_COMPONENT(TileMapInstanceComponent)

private:
    CAVE_PROP(editor = Asset, on_change = onTileMapGuidChanged)
    Guid m_tile_map_guid;

    CAVE_PROP(editor = Color)
    math::Vec4f m_tint_color = math::Vec4f::One;

    struct Cache {
        Handle<ImageAsset> image;
        Handle<TileSetAsset> tile_set_handle;
        mutable std::shared_ptr<GpuMesh> mesh;
    };

    // Non serialize
    bool m_visible;
    Handle<TileMapAsset> m_handle;
    Cache m_cache;
    uint32_t m_revision{ 0 };

    void refreshTileMapHandle();
    void onTileMapGuidChanged(const FieldChange& change);

public:
    // @TODO: better way to create data
    void createRenderData();

    bool visible() const { return m_visible; }
    const Cache& cache() const { return m_cache; }

    const auto& tileMapHandle() const { return m_handle; }

    const math::Vec4f& tintColor() const { return m_tint_color; }

    const Guid& tileMapGuid() const { return m_tile_map_guid; }

    void onDeserialized();
};

}  // namespace cave
