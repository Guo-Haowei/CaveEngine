#pragma once
#include "scene_component_base.h"

#include "engine/assets/asset_handle.h"
#include "engine/math/box.h"
#include "engine/math/geomath.h"

namespace my {

class Archive;
struct GpuMesh;
struct ImageAsset;
class SpriteAsset;
class TileMapAsset;

class TileMapRenderer : public ComponentFlagBase {
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

#if 0
struct SpriteSheet {
    std::vector<Rect> frames;

    // not serialized
    mutable const ImageAsset* texture;

    const Rect& getFrame(int index) const { return frames[index]; }
};

class TileMapComponent : public ComponentFlagBase {
public:
    void FromArray(const std::vector<std::vector<int>>& p_data);

    void SetTile(int p_x, int p_y, int p_tile_id);

    int GetTile(int p_x, int p_y) const;

    void SetDimensions(int p_width, int p_height);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }


    // @TODO: change to private
public:
    int m_width{ 0 };
    int m_height{ 0 };
    std::vector<int> m_tiles;

    // Non-serialized
    // @TODO: make it an asset
    SpriteSheet m_sprite;
};
#endif

}  // namespace my
