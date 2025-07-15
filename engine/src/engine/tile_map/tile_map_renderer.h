#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/math/box.h"
#include "engine/math/geomath.h"

namespace my {

class Archive;
struct GpuMesh;
struct ImageAsset;
class SpriteAsset;
class TileMapAsset;

class TileMapRenderer {
public:
    struct LayerCache {
        bool visible;
        uint32_t revision{ 0 };
        Handle<ImageAsset> image;
        Handle<SpriteAsset> sprite;
        mutable std::shared_ptr<GpuMesh> mesh;
    };

    // @TODO: better way
    void CreateRenderData();

    bool SetTileMap(const Guid& p_guid);

    const auto& GetLayerCache() const { return m_layer_cache; }

    // @TODO: get rid of old serailization code
    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
    static void RegisterClass();

private:
    Guid m_guid;

    // Non serialize
    Handle<TileMapAsset> m_handle;
    std::vector<LayerCache> m_layer_cache;
};

}  // namespace my
