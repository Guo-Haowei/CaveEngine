#include "DebugDrawService.h"

#include "engine/private/renderer/gpu_resource.h"

namespace cave {

using namespace ::cave::math;

void DebugDrawService::addBox2(const Vec2f& min,
                               const Vec2f& max,
                               const Vec4f& color,
                               const Mat4f* transform) {
    DebugDrawItem item;
    item.min = Vec3f(min, 0.0f);
    item.max = Vec3f(max, 0.0f);
    item.tint_color = color;
    item.texture;

    if (transform) {
        const Mat4f& m = *transform;
        Vec4f min4{ item.min, 1.0f };
        min4 = m * min4;
        Vec4f max4{ item.max, 1.0f };
        max4 = m * max4;
        item.min = min4.xyz;
        item.max = max4.xyz;
    }

    items_.emplace_back(item);
}

void DebugDrawService::addBox2Frame(const Vec2f& min,
                                    const Vec2f& max,
                                    const Vec4f& color,
                                    float thickness,
                                    const Mat4f* transform) {
    const float t = thickness;

    // Top
    addBox2({ min.x, max.y - t }, { max.x, max.y }, color, transform);
    // Bottom
    addBox2({ min.x, min.y }, { max.x, min.y + t }, color, transform);
    // Left
    addBox2({ min.x, min.y + t }, { min.x + t, max.y - t }, color, transform);
    // Right
    addBox2({ max.x - t, min.y + t }, { max.x, max.y - t }, color, transform);
}

#if 0
static void AddDebugCube(FrameData& p_framedata,
                         const AABB& p_aabb,
                         const Color& p_color,
                         const Mat4f* p_transform = nullptr) {

    const auto& min = p_aabb.Min();
    const auto& max = p_aabb.Max();

    std::vector<Vec3f> positions;
    std::vector<uint32_t> indices;
    BoxWireFrameHelper(min, max, positions, indices);

    auto& context = p_framedata.drawDebugContext;
    for (const auto& i : indices) {
        const Vec3f& pos = positions[i];
        if (p_transform) {
            const auto tmp = *p_transform * Vecf(pos, 1.0f);
            context.positions.emplace_back(Vec3f(tmp.xyz));
        } else {
            context.positions.emplace_back(Vec3f(pos));
        }
        context.colors.emplace_back(p_color);
    }
}
#endif

}  // namespace cave
