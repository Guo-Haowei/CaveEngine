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
