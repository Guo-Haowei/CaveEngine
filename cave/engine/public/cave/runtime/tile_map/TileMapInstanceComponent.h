// =============================================================================
// File: cave/runtime/tile_map/TileMapInstanceComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Box.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct GpuMesh;

class TileMapInstanceComponent {
    CAVE_COMPONENT(TileMapInstanceComponent)

private:
    CAVE_PROP(editor = Asset, tooltip = "tile map")
    Guid tile_map_id_;

    CAVE_PROP(editor = Color)
    math::Vec4f tint_color_ = math::Vec4f::One;

    struct Cache {
        Handle<ImageAsset> image;
        Handle<TileSetAsset> tile_set_handle;
        mutable std::shared_ptr<GpuMesh> mesh;
    };

    // Non serialize
    bool visible_;
    Handle<TileMapAsset> handle_;
    Cache cache_;
    uint32_t revision_{ 0 };

public:
    // @TODO: better way to create data
    void createRenderData();

    bool visible() const { return visible_; }
    const Cache& cache() const { return cache_; }

    const auto& tileMapHandle() const { return handle_; }

    void tintColor(const math::Vec4f& tint_color);
    const math::Vec4f& tintColor() const { return tint_color_; }

    // @TODO: change to camelCase
    bool SetResourceGuid(const Guid& guid);
    const Guid& GetResourceGuid() const { return tile_map_id_; }
    void OnDeserialized();
};

}  // namespace cave
