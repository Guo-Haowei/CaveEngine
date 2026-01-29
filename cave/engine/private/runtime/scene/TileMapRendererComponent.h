#pragma once
#include "engine/private/reflection/reflection.h"
#include "engine/private/assets/asset_handle.h"
#include "cave/core/math/Box.h"
#include "engine/private/core/math/geomath.h"

// @TODO: rename it to TileMapInstanceComponent

namespace cave {

struct GpuMesh;

class TileMapRendererComponent {
    CAVE_META(TileMapRendererComponent)

private:
    CAVE_PROP(editor = Asset, tooltip = "tile map")
    Guid m_tile_map_id;

    CAVE_PROP(editor = Color)
    math::Vector4f m_tint_color = math::Vector4f::One;

    struct Cache {
        Handle<ImageAsset> image;
        Handle<TileSetAsset> tile_set_handle;
        mutable std::shared_ptr<GpuMesh> mesh;
    };

    // Non serialize
    bool m_is_visible;
    Handle<TileMapAsset> m_handle;
    Cache m_cache;
    uint32_t m_revision{ 0 };

public:
    // @TODO: better way to create data
    void CreateRenderData();

    bool IsVisible() const { return m_is_visible; }
    const auto& GetCache() const { return m_cache; }

    bool SetResourceGuid(const Guid& p_guid);
    const Guid& GetResourceGuid() const { return m_tile_map_id; }

    const auto& GetTileMapHandle() const { return m_handle; }

    void SetTintColor(const math::Vector4f& p_tint_color);
    const math::Vector4f& GetTintColor() const { return m_tint_color; }

    void OnDeserialized();
};

}  // namespace cave
