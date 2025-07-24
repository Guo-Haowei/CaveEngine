#include "tile_map_renderer_component.h"

#include "engine/assets/tile_map_asset.h"
#include "engine/assets/tile_set_asset.h"
#include "engine/renderer/gpu_resource.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/graphics_manager_interface.h"

namespace cave {

void TileMapRendererComponent::SetTintColor(const Vector4f& p_tint_color) {
    m_tint_color = p_tint_color;
}

void TileMapRendererComponent::SetResourceGuid(const Guid& p_guid) {
    AssetHandle::ReplaceGuidAndHandle(AssetType::TileMap,
                                      p_guid,
                                      m_tile_map_id,
                                      m_handle.RawHandle());
}

void TileMapRendererComponent::OnDeserialized() {
    auto res = AssetRegistry::GetSingleton().FindByGuid<TileMapAsset>(m_tile_map_id);
    m_handle = std::move(res.unwrap());
}

void TileMapRendererComponent::CreateRenderData() {
    if (m_tile_map_id != m_handle.GetGuid()) {
        OnDeserialized();
    }

    auto tile_map = m_handle.Get();

    if (!tile_map) {
        return;
    }

    // @TODO: update guid
    if (m_cache.tile_set_handle.GetGuid() == Guid::Null()) {
        auto tile_set_handle = AssetRegistry::GetSingleton().FindByGuid<TileSetAsset>(tile_map->GetTileSetGuid());
        if (tile_set_handle.is_some()) {
            m_cache.tile_set_handle = std::move(tile_set_handle.unwrap_unchecked());
        }
    }

    TileSetAsset* tile_set = m_cache.tile_set_handle.Get();
    if (!tile_set) {
        return;
    }

    bool need_update = false;
    if (tile_set->IsDirty()) {
        tile_set->SetDirty(false);
        need_update = true;
    }

    if (tile_map->GetRevision() != m_revision) {
        need_update = true;
    }

    if (!need_update) {
        return;
    }

    m_cache.image = tile_set->GetHandle();

    std::vector<Vector2f> vertices;
    std::vector<Vector2f> uvs;
    std::vector<uint32_t> indices;

    const auto& chunks = tile_map->GetTiles().chunks;
    if (chunks.empty()) {
        m_visibility = false;
        return;
    }
    m_visibility = tile_map->IsVisible();

    const auto& frames = tile_set->GetFrames();

    vertices.reserve((TILE_CHUNK_SIZE * TILE_CHUNK_SIZE));
    for (const auto& [key, chunk_ptr] : chunks) {
        const int16_t offset_x = key.x * TILE_CHUNK_SIZE;
        const int16_t offset_y = key.y * TILE_CHUNK_SIZE;

        const auto& chunk = chunk_ptr->tiles;
        for (int16_t y = offset_y; y < offset_y + TILE_CHUNK_SIZE; ++y) {
            for (int16_t x = offset_x; x < offset_x + TILE_CHUNK_SIZE; ++x) {
                const TileId& tile_id = chunk[y - offset_y][x - offset_x];
                if ((int)frames.size() <= tile_id) {
                    continue;
                }

                const float s = 1.0f;
                float x0 = s * x;
                float y0 = s * y;
                float x1 = s * (x + 1);
                float y1 = s * (y + 1);
                Vector2f bottom_left{ x0, y0 };
                Vector2f bottom_right{ x1, y0 };
                Vector2f top_left{ x0, y1 };
                Vector2f top_right{ x1, y1 };
                Vector2f uv_min = frames[tile_id].GetMin();
                Vector2f uv_max = frames[tile_id].GetMax();

                // manually flip y here
                Vector2f uv0 = { uv_min.x, uv_max.y };
                Vector2f uv1 = { uv_max.x, uv_max.y };
                Vector2f uv2 = { uv_min.x, uv_min.y };
                Vector2f uv3 = { uv_max.x, uv_min.y };

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
        }
    }

    uint32_t count = (uint32_t)indices.size();

    GpuBufferDesc buffers[2];
    GpuBufferDesc buffer_desc;
    buffer_desc.type = GpuBufferType::VERTEX;
    buffer_desc.element_size = sizeof(Vector2f);
    buffer_desc.element_count = (uint32_t)vertices.size();
    buffer_desc.initial_data = vertices.data();

    buffers[0] = buffer_desc;

    buffer_desc.initial_data = uvs.data();
    buffers[1] = buffer_desc;

    GpuBufferDesc index_desc;
    index_desc.type = GpuBufferType::INDEX;
    index_desc.element_size = sizeof(uint32_t);
    index_desc.element_count = count;
    index_desc.initial_data = indices.data();

    GpuMeshDesc desc;
    desc.drawCount = count;
    desc.enabledVertexCount = 2;
    desc.vertexLayout[0] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vector2f), 0 };
    desc.vertexLayout[1] = GpuMeshDesc::VertexLayout{ 1, sizeof(Vector2f), 0 };

    // @TODO: refactor this part
    auto mesh = IGraphicsManager::GetSingleton().CreateMeshImpl(desc, 2, buffers, &index_desc);

    m_cache.mesh = *mesh;

    m_revision = tile_map->GetRevision();
}

}  // namespace cave
