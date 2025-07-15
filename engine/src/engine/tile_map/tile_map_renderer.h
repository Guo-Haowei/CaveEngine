#pragma once
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
public:
    struct Cache {
        Handle<ImageAsset> image;
        Handle<SpriteAsset> sprite;
        mutable std::shared_ptr<GpuMesh> mesh;
    };

    // @TODO: better way
    void CreateRenderData();

    bool SetTileMap(const Guid& p_guid);

    bool GetVisibility() const { return m_visibility; }
    const auto& GetCache() const { return m_cache; }

    // @TODO: get rid of old serailization code
    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
    static void RegisterClass();

private:
    Guid m_guid;
    bool m_visibility;

    // Non serialize
    Handle<TileMapAsset> m_handle;
    Cache m_cache;
    uint32_t m_revision{ 0 };
};

}  // namespace cave
