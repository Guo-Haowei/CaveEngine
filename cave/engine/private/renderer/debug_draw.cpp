#include "debug_draw.h"

#include "engine/renderer/gpu_resource.h"
#include "engine/renderer/graphics_manager.h"

namespace cave {

static constexpr float DEFAULT_Z = 0.0f;

#if 0
static void AddDebugCube(FrameData& p_framedata,
                         const AABB& p_aabb,
                         const Color& p_color,
                         const Matrix4x4f* p_transform = nullptr) {

    const auto& min = p_aabb.GetMin();
    const auto& max = p_aabb.GetMax();

    std::vector<Vector3f> positions;
    std::vector<uint32_t> indices;
    BoxWireFrameHelper(min, max, positions, indices);

    auto& context = p_framedata.drawDebugContext;
    for (const auto& i : indices) {
        const Vector3f& pos = positions[i];
        if (p_transform) {
            const auto tmp = *p_transform * Vector4f(pos, 1.0f);
            context.positions.emplace_back(Vector3f(tmp.xyz));
        } else {
            context.positions.emplace_back(Vector3f(pos));
        }
        context.colors.emplace_back(p_color);
    }
}
#endif

void DebugDraw::AddBox2(const Vector2f& p_min,
                        const Vector2f& p_max,
                        const Vector4f& p_color,
                        const Matrix4x4f* p_transform) {
    Item item;

    item.min = Vector3f(p_min, 0.0f);
    item.max = Vector3f(p_max, 0.0f);
    item.tint_color = p_color;
    item.texture;

    if (p_transform) {
        const Matrix4x4f& m = *p_transform;
        Vector4f min4{ item.min, 1.0f };
        min4 = m * min4;
        Vector4f max4{ item.max, 1.0f };
        max4 = m * max4;
        item.min = min4.xyz;
        item.max = max4.xyz;
    }

    m_items.emplace_back(item);
}

void DebugDraw::AddBox2Frame(const Vector2f& p_min,
                             const Vector2f& p_max,
                             const Vector4f& p_color,
                             const Matrix4x4f* p_transform,
                             float p_thickness) {
    const float t = p_thickness;

    // Top
    AddBox2({ p_min.x, p_max.y - t }, { p_max.x, p_max.y }, p_color, p_transform);
    // Bottom
    AddBox2({ p_min.x, p_min.y }, { p_max.x, p_min.y + t }, p_color, p_transform);
    // Left
    AddBox2({ p_min.x, p_min.y + t }, { p_min.x + t, p_max.y - t }, p_color, p_transform);
    // Right
    AddBox2({ p_max.x - t, p_min.y + t }, { p_max.x, p_max.y - t }, p_color, p_transform);
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

        positions.push_back(item.min);                                      // bottom left
        positions.push_back(Vector3f(item.max.x, item.min.y, item.min.z));  // bottom right
        positions.push_back(Vector3f(item.min.x, item.max.y, item.min.z));  // top left
        positions.push_back(item.max);                                      // top right

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
        .slot = 0,
        .element_size = sizeof(Vector3f),
        .element_count = item_count * 4,
        .initial_data = positions.data(),
    };
    buffer_descs[1] = {
        .type = GpuBufferType::VERTEX,
        .slot = 1,
        .element_size = sizeof(Vector2f),
        .element_count = item_count * 4,
        .initial_data = uvs.data(),
    };
    buffer_descs[2] = {
        .type = GpuBufferType::VERTEX,
        .slot = 2,
        .element_size = sizeof(Vector4f),
        .element_count = item_count * 4,
        .initial_data = colors.data(),
    };

    GpuBufferDesc index_desc = {
        .type = GpuBufferType::INDEX,
        .element_size = sizeof(uint32_t),
        .element_count = item_count * 6,
        .initial_data = indices.data(),
    };

    GpuMeshDesc desc;
    desc.drawCount = item_count * 6;
    desc.enabledVertexCount = 3;
    desc.vertexLayout[0] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vector3f), 0 };
    desc.vertexLayout[1] = GpuMeshDesc::VertexLayout{ 1, sizeof(Vector2f), 0 };
    desc.vertexLayout[2] = GpuMeshDesc::VertexLayout{ 2, sizeof(Vector4f), 0 };

    auto mesh = GraphicsManager::GetSingleton().CreateMeshImpl(desc, 3, buffer_descs, &index_desc);
    m_mesh = *mesh;
}

}  // namespace cave
