#include "DebugDrawService.h"

#include "engine/private/renderer/gpu_resource.h"

namespace cave {

using namespace ::cave::math;
using namespace ::cave::render;

void Canvas::addImage(GpuTexture* texture,
                                const math::Vec2f& min,
                                const math::Vec2f& max,
                                const math::Vec2f& uv_min,
                                const math::Vec2f& uv_max,
                                const math::Vec4f& tint,
                                const math::Mat4f* transform) {
    DEV_ASSERT(min.x < max.x && min.y < max.y);

    PrimShape shape{
        .type = PrimShapeType::Rect,
        .vertices = {
            PrimVert{ Vec3f(min.x, min.y, 0.0f), Vec2f(uv_min.x, uv_min.y), tint },
            PrimVert{ Vec3f(max.x, min.y, 0.0f), Vec2f(uv_max.x, uv_min.y), tint },
            PrimVert{ Vec3f(max.x, max.y, 0.0f), Vec2f(uv_max.x, uv_max.y), tint },
            PrimVert{ Vec3f(min.x, max.y, 0.0f), Vec2f(uv_min.x, uv_max.y), tint },
        },
        .tex = texture,
    };

    if (transform) {
        for (PrimVert& vert : shape.vertices) {
            vert.pos = ((*transform) * Vec4f(vert.pos, 1.0f)).xyz;
        }
    }

    m_shapes.push_back(shape);
}
void Canvas::addBox2(const Vec2f& min,
                               const Vec2f& max,
                               const Vec4f& tint,
                               const Mat4f* transform) {
    addImage(nullptr, min, max, Vec2f::Zero, Vec2f::Zero, tint, transform);
}

void Canvas::addBox2Frame(const Vec2f& min,
                                    const Vec2f& max,
                                    float thickness,
                                    const Vec4f& tint,
                                    const Mat4f* transform) {
    // @TODO: probably need to know aspect ratio to adjust thickness
    const float t = thickness;

    // Top
    addBox2({ min.x, max.y - t }, { max.x, max.y }, tint, transform);
    // Bottom
    addBox2({ min.x, min.y }, { max.x, min.y + t }, tint, transform);
    // Left
    addBox2({ min.x, min.y + t }, { min.x + t, max.y - t }, tint, transform);
    // Right
    addBox2({ max.x - t, min.y + t }, { max.x, max.y - t }, tint, transform);
}

}  // namespace cave
