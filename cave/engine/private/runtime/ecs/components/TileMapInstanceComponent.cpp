#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileMapInstanceComponent.h"

#include "cave/runtime/tile_map/TileSetAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/renderer/gpu_resource.h"

namespace cave {

using namespace math;
using namespace render;

void TileMapInstanceComponent::tintColor(const Vec4f& tint_color) {
    tint_color_ = tint_color;
}

bool TileMapInstanceComponent::SetResourceGuid(const Guid& guid) {
    return AssetHandle::replaceGuidAndHandle(AssetType::TileMap,
                                             guid,
                                             tile_map_id_,
                                             handle_.rawHandle());
}

void TileMapInstanceComponent::OnDeserialized() {
    auto res = AssetRegistry::singleton().findByGuid<TileMapAsset>(tile_map_id_);
    handle_ = std::move(res.unwrap());
}

void TileMapInstanceComponent::createRenderData() {
    if (tile_map_id_ != handle_.guid()) {
        OnDeserialized();
    }

    auto tile_map = handle_.get();

    if (!tile_map) {
        return;
    }

    visible_ = tile_map->visible();

    // @TODO: update guid
    if (cache_.tile_set_handle.guid() == Guid::Null()) {
        auto tile_set_handle = AssetRegistry::singleton().findByGuid<TileSetAsset>(tile_map->tileSetGuid());
        if (tile_set_handle.is_some()) {
            cache_.tile_set_handle = std::move(tile_set_handle.unwrap_unchecked());
        }
    }

    TileSetAsset* tile_set = cache_.tile_set_handle.get();
    if (!tile_set) {
        return;
    }

    bool need_update = false;
    if (tile_set->dirty()) {
        tile_set->dirty(false);
        need_update = true;
    }

    if (tile_map->revision() != revision_) {
        need_update = true;
    }

    if (!need_update) {
        return;
    }

    cache_.image = tile_set->handle();

    std::vector<Vec2f> vertices;
    std::vector<Vec2f> uvs;
    std::vector<uint32_t> indices;

    const auto& chunks = tile_map->tiles().chunks();
    if (chunks.empty()) {
        visible_ = false;
        return;
    }

    const auto& frames = tile_set->GetFrames();

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
                Vec2f uv_min = frames[tile_id].Min();
                Vec2f uv_max = frames[tile_id].Max();

                // manually flip y here
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

    uint32_t count = (uint32_t)indices.size();

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
    auto mesh = RenderDevice::singleton().CreateMeshImpl(desc, buffers, &index_desc);

    cache_.mesh = *mesh;

    revision_ = tile_map->revision();
}

}  // namespace cave
