#pragma once
#include "engine/reflection/reflection.h"
#include "engine/assets/asset_handle.h"
#include "engine/math/box.h"
#include "engine/math/geomath.h"

namespace cave {

class Archive;
struct GpuMesh;

class TileMapRendererComponent {
    CAVE_META(TileMapRendererComponent)

    CAVE_PROP(editor = Asset, tooltip = "tile map")
    Guid m_tile_map_id;

    CAVE_PROP(editor = Color)
    Vector4f m_tint_color = Vector4f::One;

private:
    struct Cache {
        Handle<ImageAsset> image;
        Handle<TileSetAsset> tile_set_handle;
        mutable std::shared_ptr<GpuMesh> mesh;
    };

    // Non serialize
    bool m_visibility;
    Handle<TileMapAsset> m_handle;
    Cache m_cache;
    uint32_t m_revision{ 0 };

public:
    // @TODO: better way to create data
    void CreateRenderData();

    bool GetVisibility() const { return m_visibility; }
    const auto& GetCache() const { return m_cache; }

    bool SetResourceGuid(const Guid& p_guid);
    const Guid& GetResourceGuid() const { return m_tile_map_id; }

    void SetTintColor(const Vector4f& p_tint_color);
    const Vector4f& GetTintColor() const { return m_tint_color; }

    void OnDeserialized();
    void Serialize(Archive& p_archive, uint32_t p_version);
};

}  // namespace cave
