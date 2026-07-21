#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileMapLayerComponent.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

using namespace math;
using namespace render;

void TileMapLayerComponent::setTileSetGuid(const Guid& guid) {
    if (m_tile_set == guid) {
        return;
    }

    m_tile_set = guid;
    refreshTileSetHandle();
}

void TileMapLayerComponent::refreshTileSetHandle() {
    if (m_tile_set.isNull()) {
        m_tile_set_handle = {};
        return;
    }

    auto handle = AssetRegistry::singleton().findByGuid<TileSetAsset>(m_tile_set);
    if (handle.is_some()) {
        m_tile_set_handle = std::move(handle.unwrap_unchecked());
    }
}

void TileMapLayerComponent::onTileSetGuidChanged(const FieldChange& change) {
    DEV_ASSERT((*(const Guid*)(change.old_value)) != (*(const Guid*)(change.new_value)));
    DEV_ASSERT(change.object == this);
    DEV_ASSERT(change.field->id == CAVE_SID("tile_set"));

    refreshTileSetHandle();
}

void TileMapLayerComponent::onDeserialized() {
    refreshTileSetHandle();
}

void TileMapInstanceComponent::refreshTileMapHandle() {
    m_revision = 0;
    m_handle.invalidate();
    m_layers.clear();

    if (m_tile_map_guid.isNull()) {
        return;
    }

    auto res = AssetRegistry::singleton().findByGuid<TileMapAsset>(m_tile_map_guid);
    if (res.is_some()) {
        m_handle = std::move(res.unwrap_unchecked());
    }
}

void TileMapInstanceComponent::onTileMapGuidChanged(const FieldChange& change) {
    DEV_ASSERT((*(const Guid*)(change.old_value)) != (*(const Guid*)(change.new_value)));
    DEV_ASSERT(change.object == this);
    DEV_ASSERT(change.field->id == CAVE_SID("tile_map_guid"));

    refreshTileMapHandle();
}

void TileMapInstanceComponent::onDeserialized() {
    refreshTileMapHandle();
}

bool TileMapInstanceComponent::updateLayer(const TileMapLayer& layer, LayerCache& layer_cache) {
    auto tile_set_handle = AssetRegistry::singleton().findByGuid<TileSetAsset>(layer.tileSetGuid());
    if (tile_set_handle) {
        layer_cache.tile_set = std::move(tile_set_handle.unwrap_unchecked());
    } else {
        layer_cache.tile_set.invalidate();
    }

    TileSetAsset* tile_set = layer_cache.tile_set.get();
    if (!DEV_VERIFY(tile_set)) {
        return true;
    }

    const auto& chunks = layer.chunks().chunks();
    if (chunks.empty()) {
        return true;
    }

    auto collect_tiles = [&]() {
        for (const auto& [key, chunk] : chunks) {
            const int16_t offset_x = key.x * kTileChunkSize;
            const int16_t offset_y = key.y * kTileChunkSize;

            for (int16_t y = offset_y; y < offset_y + kTileChunkSize; ++y) {
                for (int16_t x = offset_x; x < offset_x + kTileChunkSize; ++x) {
                    const TileId& tile_id = chunk->at(x - offset_x, y - offset_y);
                    const auto* definition = tile_set->getTileDefinition(tile_id);
                    if (definition) {
                        layer_cache.tiles.emplace_back(x, y, tile_id, 0.0f);
                    }
                }
            }
        }
    };

    collect_tiles();

    layer_cache.visible = layer.visible();
    layer_cache.z_index = layer.zIndex();
    layer_cache.image = tile_set->handle();

    return true;
}

void TileMapInstanceComponent::createRenderData() {
    if (m_tile_map_guid != m_handle.guid()) {
        onDeserialized();
    }

    auto tile_map = m_handle.get();

    if (!tile_map) {
        return;
    }

    if (m_revision == tile_map->revision()) {
        DEV_ASSERT(m_revision <= tile_map->revision());
        return;
    }

    m_layers.clear();
    m_layers.resize(tile_map->layers().size());

    int counter = 0;
    for (const TileMapLayer& layer : tile_map->layers()) {
        if (!updateLayer(layer, m_layers[counter++])) {
            return;
        }
    }

    m_revision = tile_map->revision();
}

}  // namespace cave
