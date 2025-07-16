#pragma once
#include "engine/reflection/reflection.h"
#include "engine/assets/asset_handle.h"
#include "engine/math/box.h"
#include "engine/math/geomath.h"

namespace cave {

class Archive;
struct GpuMesh;
struct ImageAsset;
class SpriteAsset;
class TileMapAsset;

class TileMapRenderer {
    CAVE_META(TileMapRenderer)

    CAVE_PROP(type = guid, tooltip = "tile map")
    Guid m_tile_map;

private:
    struct Cache {
        Handle<ImageAsset> image;
        Handle<SpriteAsset> sprite;
        mutable std::shared_ptr<GpuMesh> mesh;
    };

    // Non serialize
    bool m_visibility;
    Handle<TileMapAsset> m_handle;
    Cache m_cache;
    uint32_t m_revision{ 0 };

public:
    // @TODO: better way
    void CreateRenderData();

    bool SetTileMap(const Guid& p_guid);

    bool GetVisibility() const { return m_visibility; }
    const auto& GetCache() const { return m_cache; }

    const Guid& GetGuid() const { return m_tile_map; }

    // @TODO: get rid of old serailization code
    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
};

}  // namespace cave
