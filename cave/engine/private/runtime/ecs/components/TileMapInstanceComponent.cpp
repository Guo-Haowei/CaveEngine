#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"

#include "cave/runtime/tile_map/TileSetAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/renderer/gpu_resource.h"

namespace cave {

using namespace math;
using namespace render;

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

bool TileMapInstanceComponent::updateLayer(const TileMapLayer& layer, Cache& cache) {
    auto tile_set_handle = AssetRegistry::singleton().findByGuid<TileSetAsset>(layer.tileSetGuid());
    if (tile_set_handle) {
        cache.tile_set_handle = std::move(tile_set_handle.unwrap_unchecked());
    } else {
        cache.tile_set_handle.invalidate();
    }

    TileSetAsset* tile_set = cache.tile_set_handle.get();
    if (!DEV_VERIFY(tile_set)) {
        cache.mesh = nullptr;
        return true;
    }

    cache.image = tile_set->handle();

    Vector<Vec2f> vertices;
    Vector<Vec2f> uvs;
    Vector<uint32_t> indices;

    const auto& chunks = layer.chunks().chunks();
    if (chunks.empty()) {
        cache.mesh = nullptr;
        return true;
    }

    const auto& frames = tile_set->frames();

    vertices.reserve((kTileChunkSize * kTileChunkSize));
    for (const auto& [key, chunk] : chunks) {
        const int16_t offset_x = key.x * kTileChunkSize;
        const int16_t offset_y = key.y * kTileChunkSize;

        for (int16_t y = offset_y; y < offset_y + kTileChunkSize; ++y) {
            for (int16_t x = offset_x; x < offset_x + kTileChunkSize; ++x) {
                const TileId& tile_id = chunk->at(x - offset_x, y - offset_y);
                if ((int)frames.size() <= tile_id) {
                    continue;
                }

                const float s = 1.0f;
                float x0 = s * x;
                float y0 = s * y;
                float x1 = s * (x + 1);
                float y1 = s * (y + 1);
                Vec2f bottom_left{ x0, y0 };
                Vec2f bottom_right{ x1, y0 };
                Vec2f top_left{ x0, y1 };
                Vec2f top_right{ x1, y1 };
                Vec2f uv_min = frames[tile_id].min();
                Vec2f uv_max = frames[tile_id].max();

                Vec2f uv0 = { uv_min.x, uv_max.y };
                Vec2f uv1 = { uv_max.x, uv_max.y };
                Vec2f uv2 = { uv_min.x, uv_min.y };
                Vec2f uv3 = { uv_max.x, uv_min.y };

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

    const uint32_t count = static_cast<uint32_t>(indices.size());
    if (count == 0) {
        cache.mesh = nullptr;
        return true;
    }

    std::array<GpuBufferDesc, 2> buffers;
    GpuBufferDesc buffer_desc;
    buffer_desc.type = GpuBufferType::Vertex;
    buffer_desc.element_size = sizeof(Vec2f);
    buffer_desc.element_count = (uint32_t)vertices.size();
    buffer_desc.initial_data = vertices.data();

    buffers[0] = buffer_desc;

    buffer_desc.initial_data = uvs.data();
    buffers[1] = buffer_desc;

    GpuBufferDesc index_desc;
    index_desc.type = GpuBufferType::Index;
    index_desc.element_size = sizeof(uint32_t);
    index_desc.element_count = count;
    index_desc.initial_data = indices.data();

    GpuMeshDesc desc;
    desc.drawCount = count;
    desc.enabledVertexCount = 2;
    desc.vertexLayout[0] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vec2f), 0 };
    desc.vertexLayout[1] = GpuMeshDesc::VertexLayout{ 1, sizeof(Vec2f), 0 };

    // @TODO: refactor this part
    // @NOTE: shouldn't call RenderDevice here
    auto mesh = RenderDevice::singleton().CreateMeshImpl(desc,
                                                         buffers,
                                                         &index_desc);

    cache.mesh = mesh.value_or(nullptr);
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
