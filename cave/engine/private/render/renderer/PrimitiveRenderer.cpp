#include "PrimitiveRenderer.h"

namespace cave::render {

#if 0
	static std::shared_ptr<GpuMesh> DebugDrawItemsBuffer(render::IRenderDevice& device,
                                                     std::span<const DebugDrawItem> items_) {
    const uint32_t item_count = static_cast<uint32_t>(items_.size());
    if (item_count == 0) {
        return nullptr;
    }

    std::vector<uint32_t> indices;
    std::vector<Vec3f> positions;
    std::vector<Vec2f> uvs;
    std::vector<Vec4f> colors;

    indices.reserve(item_count * 6);
    positions.reserve(item_count * 4);
    uvs.reserve(item_count * 4);
    colors.reserve(item_count * 4);

    for (const auto& item : items_) {
        const uint32_t offset = static_cast<uint32_t>(positions.size());

        positions.push_back(item.min);                                   // bottom left
        positions.push_back(Vec3f(item.max.x, item.min.y, item.min.z));  // bottom right
        positions.push_back(Vec3f(item.min.x, item.max.y, item.min.z));  // top left
        positions.push_back(item.max);                                   // top right

        colors.push_back(item.tint_color);
        colors.push_back(item.tint_color);
        colors.push_back(item.tint_color);
        colors.push_back(item.tint_color);

        uvs.push_back(Vec2f(0, 0));
        uvs.push_back(Vec2f(1, 0));
        uvs.push_back(Vec2f(0, 1));
        uvs.push_back(Vec2f(1, 1));

        indices.push_back(0 + offset);
        indices.push_back(1 + offset);
        indices.push_back(3 + offset);

        indices.push_back(0 + offset);
        indices.push_back(3 + offset);
        indices.push_back(2 + offset);
    }

    DEV_ASSERT((uint32_t)indices.size() == item_count * 6);
    std::array<GpuBufferDesc, 3> buffer_descs;
    buffer_descs[0] = {
        .type = GpuBufferType::Vertex,
        .slot = 0,
        .element_size = sizeof(Vec3f),
        .element_count = item_count * 4,
        .initial_data = positions.data(),
    };
    buffer_descs[1] = {
        .type = GpuBufferType::Vertex,
        .slot = 1,
        .element_size = sizeof(Vec2f),
        .element_count = item_count * 4,
        .initial_data = uvs.data(),
    };
    buffer_descs[2] = {
        .type = GpuBufferType::Vertex,
        .slot = 2,
        .element_size = sizeof(Vec4f),
        .element_count = item_count * 4,
        .initial_data = colors.data(),
    };

    GpuBufferDesc index_desc = {
        .type = GpuBufferType::Index,
        .element_size = sizeof(uint32_t),
        .element_count = item_count * 6,
        .initial_data = indices.data(),
    };

    GpuMeshDesc desc;
    desc.drawCount = item_count * 6;
    desc.enabledVertexCount = 3;
    desc.vertexLayout[0] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vec3f), 0 };
    desc.vertexLayout[1] = GpuMeshDesc::VertexLayout{ 1, sizeof(Vec2f), 0 };
    desc.vertexLayout[2] = GpuMeshDesc::VertexLayout{ 2, sizeof(Vec4f), 0 };

    return *(device.CreateMeshImpl(desc, buffer_descs, &index_desc));
}

#endif

}  // namespace cave::render
