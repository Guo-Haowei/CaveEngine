#include "tile_map_renderer.h"

#include "tile_map_asset.h"

#include "engine/assets/sprite_asset.h"
#include "engine/renderer/gpu_resource.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/graphics_manager_interface.h"

namespace cave {

void TileMapRenderer::SetTintColor(const Vector4f& p_tint_color) {
    m_tint_color = p_tint_color;
}

bool TileMapRenderer::SetTileMap(const Guid& p_guid) {
    return AssetHandle::ReplaceGuidAndHandle(AssetType::TileMap,
                                             p_guid,
                                             m_tile_map,
                                             m_handle.RawHandle());
}

void TileMapRenderer::OnDeserialized() {
    auto res = AssetRegistry::GetSingleton().FindByGuid<TileMapAsset>(m_tile_map);
    m_handle = std::move(res.unwrap());
}

void TileMapRenderer::CreateRenderData() {
    if (m_tile_map != m_handle.GetGuid()) {
        OnDeserialized();
    }

    auto tile_map = m_handle.Get();

    if (!tile_map) {
        return;
    }

    if (tile_map->GetRevision() == m_revision) {
        return;
    }

    // @TODO: update guid
    if (m_cache.sprite.GetGuid() == Guid::Null()) {
        auto sprite_handle = AssetRegistry::GetSingleton().FindByGuid<SpriteAsset>(tile_map->GetSpriteGuid());
        if (sprite_handle.is_some()) {
            m_cache.sprite = std::move(sprite_handle.unwrap_unchecked());
        }
    }

    SpriteAsset* sprite = m_cache.sprite.Get();
    if (!sprite) {
        return;
    }

    m_cache.image = sprite->GetHandle();

    std::vector<Vector2f> vertices;
    std::vector<Vector2f> uvs;
    std::vector<uint32_t> indices;

    const auto& tiles = tile_map->GetTiles();
    if (tiles.empty()) {
        m_visibility = false;
        return;
    }
    m_visibility = tile_map->IsVisible();

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
#if 1
        const auto& frames = sprite->GetFrames();
        DEV_ASSERT((int)frames.size() > tile);
        Vector2f uv_min = frames[tile - 1].GetMin();
        Vector2f uv_max = frames[tile - 1].GetMax();
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

    m_cache.mesh = *mesh;

    m_revision = tile_map->GetRevision();
}

void TileMapRenderer::Serialize(Archive& p_archive, uint32_t p_version) {
    unused(p_archive);
    unused(p_version);
    CRASH_NOW();
}

}  // namespace cave
