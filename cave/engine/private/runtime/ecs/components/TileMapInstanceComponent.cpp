#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"

#include "cave/runtime/tile_map/TileSetAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/renderer/gpu_resource.h"

namespace cave {

using namespace math;
using namespace render;

namespace {

void AppendTileQuad(int16_t x,
                    int16_t y,
                    const math::Box2& frame,
                    Vector<Vec2f>& vertices,
                    Vector<Vec2f>& uvs,
                    Vector<uint32_t>& indices) {
    const float s = 1.0f;
    float x0 = s * x;
    float y0 = s * y;
    float x1 = s * (x + 1);
    float y1 = s * (y + 1);

    Vec2f bottom_left{ x0, y0 };
    Vec2f bottom_right{ x1, y0 };
    Vec2f top_left{ x0, y1 };
    Vec2f top_right{ x1, y1 };
    Vec2f uv_min = frame.min();
    Vec2f uv_max = frame.max();

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

Ref<GpuMesh> CreateGpuMesh(std::span<const Vec2f> vertices,
                   std::span<const Vec2f> uvs,
                   std::span<const uint32_t> indices) {

    const uint32_t count = static_cast<uint32_t>(indices.size());
    if (count == 0) {
        return nullptr;
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
    return mesh.value_or(nullptr);
}

}  // namespace

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
    // @TODO: use canvas and get rid of this shit
    layer_cache.mesh = nullptr;

    auto tile_set_handle = AssetRegistry::singleton().findByGuid<TileSetAsset>(layer.tileSetGuid());
    if (tile_set_handle) {
        layer_cache.tile_set_handle = std::move(tile_set_handle.unwrap_unchecked());
    } else {
        layer_cache.tile_set_handle.invalidate();
    }

    TileSetAsset* tile_set = layer_cache.tile_set_handle.get();
    if (!DEV_VERIFY(tile_set)) {
        return true;
    }

    const auto& chunks = layer.chunks().chunks();
    if (chunks.empty()) {
        return true;
    }

    struct TileInfo {
        int16_t x, y;
        const TileDefinition* definition;
    };

    Vector<TileInfo> static_info;
    Vector<TileInfo> animated_info;

    auto collect_tiles = [&]() {
        for (const auto& [key, chunk] : chunks) {
            const int16_t offset_x = key.x * kTileChunkSize;
            const int16_t offset_y = key.y * kTileChunkSize;

            for (int16_t y = offset_y; y < offset_y + kTileChunkSize; ++y) {
                for (int16_t x = offset_x; x < offset_x + kTileChunkSize; ++x) {
                    const TileId& tile_id = chunk->at(x - offset_x, y - offset_y);
                    const auto* definition = tile_set->getTileDefinition(tile_id);
                    if (definition) {
                        if (definition->animation.empty()) {
                            static_info.emplace_back(x, y, definition);
                        } else {
                            animated_info.emplace_back(x, y, definition);
                        }
                    }
                }
            }
        }
    };

    collect_tiles();

    const auto& frames = tile_set->frames();

    layer_cache.visible = layer.visible();
    layer_cache.z_index = layer.zIndex();
    layer_cache.image = tile_set->handle();

    if (!static_info.empty())
    {
        Vector<Vec2f> vertices;
        Vector<Vec2f> uvs;
        Vector<uint32_t> indices;

        for (const auto& tile : static_info) {
            const auto atlas_index = tile.definition->id;
            if (DEV_VERIFY(atlas_index < frames.size())) {
                AppendTileQuad(tile.x, tile.y, frames[atlas_index], vertices, uvs, indices);
            }
        }

        layer_cache.mesh = CreateGpuMesh(vertices, uvs, indices);
    }

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
