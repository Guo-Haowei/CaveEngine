#include "cave/runtime/tile_map/TileMapLayerComponent.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

using namespace math;
using namespace render;

bool TileMapLayerComponent::setCell(TileCoord coord, TileCell cell) {
    const bool changed = cell.empty() ? m_chunks.removeCell(coord) : m_chunks.setCell(coord, cell);
    if (!changed) return false;

    updateCachedTile(coord);
    resolveTerrainAround(coord);
    return true;
}

bool TileMapLayerComponent::removeCell(TileCoord coord) {
    if (!m_chunks.removeCell(coord)) return false;

    updateCachedTile(coord);
    resolveTerrainAround(coord);
    return true;
}

void TileMapLayerComponent::resolveTerrainCell(TileCoord coord) {
    const TileSetAsset* tile_set = m_tile_set_handle.get();
    if (!tile_set) return;

    auto current = m_chunks.cellAt(coord);
    if (current.is_none()) return;

    TileCell cell = current.unwrap_unchecked();
    if (!cell.hasTerrain()) return;

    const uint16_t mask = buildTerrainMask(coord, cell.terrain_id);
    cell.tile_id = tile_set->resolveTerrain(cell.terrain_id, mask).unwrap_or(TileId::null());
    m_chunks.setCell(coord, cell);
    updateCachedTile(coord);
}

void TileMapLayerComponent::resolveTerrainAround(TileCoord center) {
    for (int16_t y = -1; y <= 1; ++y) {
        for (int16_t x = -1; x <= 1; ++x) {
            resolveTerrainCell(TileCoord{ static_cast<int16_t>(center.x + x), static_cast<int16_t>(center.y + y) });
        }
    }
}

void TileMapLayerComponent::updateCachedTile(TileCoord coord) {
    std::erase_if(m_tile_cache, [coord](const TileCache& tile) {
        return tile.x == coord.x && tile.y == coord.y;
    });

    const TileSetAsset* tile_set = m_tile_set_handle.get();
    if (!tile_set) return;

    auto cell = m_chunks.cellAt(coord);
    if (cell.is_none() || !cell.unwrap_unchecked().hasTile()) return;

    const TileCell value = cell.unwrap_unchecked();
    if (!tile_set->findTileDefinition(value.tile_id.value)) return;

    m_tile_cache.emplace_back(coord.x, coord.y, value.tile_id, 0.0f);
}

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

    updateAllCachedTile();
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

#if 0
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
#endif

void TileMapLayerComponent::updateAllCachedTile() {
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
    auto get_terrain = [this, terrain_id](TileCoord coord) {
        auto cell = m_chunks.cellAt(coord);
        return cell.is_none() ? false : cell.unwrap_unchecked().terrain_id == terrain_id;
    };

    const auto N = coord + TileCoord{ 0, +1 };
    const auto S = coord + TileCoord{ 0, -1 };
    const auto W = coord + TileCoord{ -1, 0 };
    const auto E = coord + TileCoord{ +1, 0 };

    const auto NE = N + TileCoord{ +1, 0 };
    const auto NW = N + TileCoord{ -1, 0 };
    const auto SE = S + TileCoord{ +1, 0 };
    const auto SW = S + TileCoord{ -1, 0 };

    const bool c = true;
    // const bool c = get_terrain(coord);
    const bool n = get_terrain(N);
    const bool s = get_terrain(S);
    const bool w = get_terrain(W);
    const bool e = get_terrain(E);
    const bool ne = n && e && get_terrain(NE);
    const bool nw = n && w && get_terrain(NW);
    const bool se = s && e && get_terrain(SE);
    const bool sw = s && w && get_terrain(SW);

    uint16_t mask = 0;

    mask |= nw ? (1u << 0) : 0;
    mask |= n ? (1u << 1) : 0;
    mask |= ne ? (1u << 2) : 0;

    mask |= w ? (1u << 3) : 0;
    mask |= c ? (1u << 4) : 0;
    mask |= e ? (1u << 5) : 0;

    mask |= sw ? (1u << 6) : 0;
    mask |= s ? (1u << 7) : 0;
    mask |= se ? (1u << 8) : 0;

    return mask;
}

}  // namespace cave
