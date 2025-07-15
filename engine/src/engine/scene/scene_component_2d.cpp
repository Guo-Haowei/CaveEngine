#include "scene_component_2d.h"

#include "engine/assets/sprite_asset.h"
#include "engine/assets/tile_map_asset.h"
#include "engine/renderer/gpu_resource.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/graphics_manager_interface.h"

namespace my {

bool TileMapRenderer::SetTileMap(const Guid& p_guid) {
    if (p_guid == m_guid) {
        return false;
    }

    m_guid = p_guid;
    auto res = AssetRegistry::GetSingleton().FindByGuid<TileMapAsset>(p_guid);
    if (!res) {
        return false;
    }

    auto handle = *res;
    return true;
}

void TileMapRenderer::CreateRenderData() {
    if (m_guid != m_handle.GetGuid()) {
        auto res = AssetRegistry::GetSingleton().FindByGuid<TileMapAsset>(m_guid);
        m_handle = std::move(*res);
    }

    auto tile_map_asset = m_handle.Get();

    if (!tile_map_asset) {
        return;
    }

    const auto& layers = tile_map_asset->GetAllLayers();
    const int layer_count = static_cast<int>(layers.size());
    if (m_layer_cache.size() != layers.size()) {
        m_layer_cache.clear();
        m_layer_cache.resize(layer_count);
    }

    // @TODO: multi layer
    for (int layer_id = 0; layer_id < layer_count; ++layer_id) {
        const auto& layer = layers[layer_id];
        auto& layer_cache = m_layer_cache[layer_id];
        if (layer.GetRevision() == layer_cache.revision) {
            break;
        }

        // @TODO: update guid
        if (layer_cache.image.GetGuid() == Guid::Null()) {
            auto sprite_handle = AssetRegistry::GetSingleton().FindByGuid<SpriteAsset>(layer.GetSpriteGuid());
            if (sprite_handle) {
                SpriteAsset* sprite = sprite_handle->Get();
                if (sprite) {
                    layer_cache.image = sprite->GetHandle();
                }
            }
        }

        std::vector<Vector2f> vertices;
        std::vector<Vector2f> uvs;
        std::vector<uint32_t> indices;

        const auto& tiles = layer.GetTiles();
        if (tiles.empty()) {
            continue;
        }

        vertices.reserve(tiles.size() * 4);
        for (const auto& [key, tile] : tiles) {
            const int16_t x = key.x;
            const int16_t y = key.y;

            const float s = 1.0f;
            float x0 = s * x;
            float y0 = s * y;
            float x1 = s * (x + 1);
            float y1 = s * (y + 1);
            Vector2f bottom_left{ x0, y0 };
            Vector2f bottom_right{ x1, y0 };
            Vector2f top_left{ x0, y1 };
            Vector2f top_right{ x1, y1 };
#if 0
            Vector2f uv_min = m_sprite.frames[tile - 1].GetMin();
            Vector2f uv_max = m_sprite.frames[tile - 1].GetMax();
#else
            Vector2f uv_min = Vector2f::Zero;
            Vector2f uv_max = Vector2f::One;
#endif
            Vector2f uv0 = uv_min;
            Vector2f uv1 = { uv_max.x, uv_min.y };
            Vector2f uv2 = { uv_min.x, uv_max.y };
            Vector2f uv3 = uv_max;

            const uint32_t offset = (uint32_t)vertices.size();
            vertices.push_back(bottom_left);
            vertices.push_back(bottom_right);
            vertices.push_back(top_left);
            vertices.push_back(top_right);

            uvs.push_back(uv0);
            uvs.push_back(uv1);
            uvs.push_back(uv2);
            uvs.push_back(uv3);

            indices.push_back(0 + offset);
            indices.push_back(1 + offset);
            indices.push_back(3 + offset);

            indices.push_back(0 + offset);
            indices.push_back(3 + offset);
            indices.push_back(2 + offset);
        }

        uint32_t count = (uint32_t)indices.size();

        GpuBufferDesc buffers[2];
        GpuBufferDesc buffer_desc;
        buffer_desc.type = GpuBufferType::VERTEX;
        buffer_desc.elementSize = sizeof(Vector2f);
        buffer_desc.elementCount = (uint32_t)vertices.size();
        buffer_desc.initialData = vertices.data();

        buffers[0] = buffer_desc;

        buffer_desc.initialData = uvs.data();
        buffers[1] = buffer_desc;

        GpuBufferDesc index_desc;
        index_desc.type = GpuBufferType::INDEX;
        index_desc.elementSize = sizeof(uint32_t);
        index_desc.elementCount = count;
        index_desc.initialData = indices.data();

        GpuMeshDesc desc;
        desc.drawCount = count;
        desc.enabledVertexCount = 2;
        desc.vertexLayout[0] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vector2f), 0 };
        desc.vertexLayout[1] = GpuMeshDesc::VertexLayout{ 1, sizeof(Vector2f), 0 };

        // @TODO: refactor this part
        auto mesh = IGraphicsManager::GetSingleton().CreateMeshImpl(desc, 2, buffers, &index_desc);

        layer_cache.mesh = *mesh;
        layer_cache.revision = layer.GetRevision();
    }
}

#if 0
void TileMapComponent::FromArray(const std::vector<std::vector<int>>& p_data) {
    m_width = static_cast<int>(p_data[0].size());
    m_height = static_cast<int>(p_data.size());

    m_tiles.resize(m_width * m_height);
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            m_tiles[y * m_width + x] = p_data[m_height - 1 - y][x];
            // m_tiles[y * m_width + x] = p_data[y][x];
        }
    }

    SetDirty();
}

void TileMapComponent::SetTile(int p_x, int p_y, int p_tile_id) {
    if (p_x >= 0 && p_x < m_width && p_y >= 0 && p_y < m_height) {
        int& old = m_tiles[p_y * m_width + p_x];
        if (old != p_tile_id) {
            old = p_tile_id;
            SetDirty();
        }
    }
}

int TileMapComponent::GetTile(int p_x, int p_y) const {
    if (p_x >= 0 && p_x < m_width && p_y >= 0 && p_y < m_height) {
        return m_tiles[p_y * m_width + p_x];
    }
    return 0;
}

void TileMapComponent::SetDimensions(int p_width, int p_height) {
    m_width = p_width;
    m_height = p_height;
    m_tiles.resize(p_width * p_height);
}
#endif

}  // namespace my
