#include "cave/runtime/tile_map/TileMapLayerComponent.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/assets/ImageAsset.h"
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
        m_image_handle = {};
        return;
    }

    auto tile_set_handle = AssetRegistry::singleton().findByGuid<TileSetAsset>(m_tile_set);
    if (!tile_set_handle) {
        return;
    }

    m_tile_set_handle = std::move(tile_set_handle.unwrap_unchecked());
    const TileSetAsset* tile_set = m_tile_set_handle.get();
    if (!tile_set) {
        m_tile_set_handle = {};
        return;
    }

    auto image_handle = AssetRegistry::singleton().findByGuid<ImageAsset>(tile_set->imageGuid());
    if (!image_handle) {
        m_tile_set_handle = {};
        return;
    }

    m_image_handle = std::move(image_handle.unwrap_unchecked());

    resolveAllTerrain();
    updateTileCache();
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

void TileMapLayerComponent::resolveAllTerrain() {
    const TileSetAsset* tile_set = m_tile_set_handle.get();
    if (!tile_set) {
        return;
    }

    for (const auto& [key, chunk] : m_chunks.chunks()) {
        const int16_t offset_x = key.x * kTileChunkSize;
        const int16_t offset_y = key.y * kTileChunkSize;

        for (int16_t y = offset_y; y < offset_y + kTileChunkSize; ++y) {
            for (int16_t x = offset_x; x < offset_x + kTileChunkSize; ++x) {
                TileCell& cell = chunk->at(x - offset_x, y - offset_y);
                if (!cell.hasTerrain()) {
                    continue;
                }

                if (!cell.hasTerrain()) {
                    continue;
                }

                const TileCoord coord{ x, y };
                const uint16_t terrain_mask = buildTerrainMask(coord, cell.terrain_id);
                cell.tile_id = tile_set->resolveTerrain(cell.terrain_id, terrain_mask).unwrap_or(cell.tile_id);
            }
        }
    }
}

void TileMapLayerComponent::updateTileCache() {
    m_tile_cache.clear();

    const TileSetAsset* tile_set = m_tile_set_handle.get();
    if (!tile_set) {
        return;
    }

    for (const auto& [key, chunk] : m_chunks.chunks()) {
        const int16_t offset_x = key.x * kTileChunkSize;
        const int16_t offset_y = key.y * kTileChunkSize;

        for (int16_t y = offset_y; y < offset_y + kTileChunkSize; ++y) {
            for (int16_t x = offset_x; x < offset_x + kTileChunkSize; ++x) {
                const TileCell& cell = chunk->at(x - offset_x, y - offset_y);
                const auto* definition = tile_set->findTileDefinition(cell.tile_id.value);
                if (definition) {
                    m_tile_cache.emplace_back(x, y, cell.tile_id, 0.0f);
                }
            }
        }
    }
}

uint16_t TileMapLayerComponent::buildTerrainMask(TileCoord coord, TerrainId terrain_id) const {
    uint16_t mask = 0;

    for (int16_t y = 0; y < 3; ++y) {
        for (int16_t x = 0; x < 3; ++x) {
            const TileCoord offset{ x - 1, y - 1 };

            const TileCoord neighbor = coord + offset;
            auto cell = m_chunks.cellAt(neighbor);

            if (!cell || cell.unwrap_unchecked().terrain_id != terrain_id) {
                continue;
            }

            // @TODO: fix this part
            const auto bit_index = (2 - y) * 3 + x;
            mask |= static_cast<uint16_t>(1u << bit_index);
        }
    }

    return mask;
}

}  // namespace cave
