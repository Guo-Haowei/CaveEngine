#include "debug_draw.h"

#include "engine/renderer/gpu_resource.h"
#include "engine/runtime/graphics_manager_interface.h"

namespace cave {

static constexpr float DEFAULT_Z = 0.0f;

void DebugDraw::AddRect(const Vector3f& p_center,
                        const Vector3f& p_half,
                        const Vector4f& p_color,
                        const Matrix4x4f* p_transform) {
    Item item;

    item.min = p_center - p_half;
    item.max = p_center + p_half;
    item.tint_color = p_color;
    item.texture;

    if (p_transform) {
        const Matrix4x4f& m = *p_transform;
        Vector4f min4{ item.min, 1.0f };
        min4 = m * min4;
        Vector4f max4{ item.min, 1.0f };
        max4 = m * max4;
        item.min = min4.xyz;
        item.max = max4.xyz;
    }

    m_items.emplace_back(item);
}

void DebugDraw::Batch() {
    const uint32_t item_count = static_cast<uint32_t>(m_items.size());
    if (item_count == 0) {
        return;
    }

    std::vector<uint32_t> indices;
    std::vector<Vector3f> positions;
    std::vector<Vector2f> uvs;
    std::vector<Vector4f> colors;

    indices.reserve(item_count * 6);
    positions.reserve(item_count * 4);
    uvs.reserve(item_count * 4);
    colors.reserve(item_count * 4);

    for (const auto& item : m_items) {
        const uint32_t offset = static_cast<uint32_t>(positions.size());

        positions.push_back(item.min);
        positions.push_back(Vector3f(item.min.x, item.max.y, item.min.z));
        positions.push_back(Vector3f(item.max.x, item.min.y, item.min.z));
        positions.push_back(item.max);

        colors.push_back(item.tint_color);
        colors.push_back(item.tint_color);
        colors.push_back(item.tint_color);
        colors.push_back(item.tint_color);

        uvs.push_back(Vector2f(0, 0));
        uvs.push_back(Vector2f(1, 0));
        uvs.push_back(Vector2f(0, 1));
        uvs.push_back(Vector2f(1, 1));

        indices.push_back(0 + offset);
        indices.push_back(1 + offset);
        indices.push_back(3 + offset);

        indices.push_back(0 + offset);
        indices.push_back(3 + offset);
        indices.push_back(2 + offset);
    }

    DEV_ASSERT((uint32_t)indices.size() == item_count * 6);
    GpuBufferDesc buffer_descs[3];
    buffer_descs[0] = {
        .type = GpuBufferType::VERTEX,
        .dynamic = false,
        .slot = 0,
        .element_size = sizeof(Vector3f),
        .element_count = item_count * 4,
        .offset = 0,
        .initial_data = positions.data(),
    };
    buffer_descs[1] = {
        .type = GpuBufferType::VERTEX,
        .dynamic = false,
        .slot = 1,
        .element_size = sizeof(Vector2f),
        .element_count = item_count * 4,
        .offset = 1,
        .initial_data = uvs.data(),
    };
    buffer_descs[1] = {
        .type = GpuBufferType::VERTEX,
        .dynamic = false,
        .slot = 1,
        .element_size = sizeof(Vector4f),
        .element_count = item_count * 4,
        .offset = 2,
        .initial_data = colors.data(),
    };

    GpuBufferDesc index_desc = {
        .type = GpuBufferType::INDEX,
        .dynamic = false,
        .slot = 0,
        .element_size = sizeof(uint32_t),
        .element_count = item_count * 6,
        .offset = 0,
        .initial_data = indices.data(),
    };

    GpuMeshDesc desc;
    desc.drawCount = item_count * 6;
    desc.enabledVertexCount = 3;
    desc.vertexLayout[0] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vector3f), 0 };
    desc.vertexLayout[1] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vector2f), 0 };
    desc.vertexLayout[2] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vector4f), 0 };

    auto mesh = IGraphicsManager::GetSingleton().CreateMeshImpl(desc, 3, buffer_descs, &index_desc);
    m_mesh = *mesh;
}

}  // namespace cave
