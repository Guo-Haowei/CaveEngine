#include "Canvas.h"

#include "engine/private/renderer/gpu_resource.h"

namespace cave {

using namespace ::cave::math;
using namespace ::cave::render;

bool Canvas::canSubmit() const {
    DEV_ASSERT_MSG(m_can_submit, "can't submit after endFrame()");
    DEV_ASSERT_MSG(!m_stack.empty(), "no view pushed");
    DEV_ASSERT_MSG(m_current_idx != kInvalidIdx, "invalid canvas bucket");

    return m_can_submit &&
           !m_stack.empty() &&
           m_current_idx != kInvalidIdx;
}

void Canvas::beginFrame() {
    m_stack.clear();
    m_buckets.clear();
    m_lookup.clear();

    m_current_idx = kInvalidIdx;
    m_can_submit = true;
}

void Canvas::endFrame() {
    DEV_ASSERT(m_stack.empty());
    DEV_ASSERT(m_current_idx == kInvalidIdx);
    m_can_submit = false;
}

void Canvas::pushView(ViewId view_id) {
    int idx = static_cast<int>(m_buckets.size());
    auto [it, inserted] = m_lookup.try_emplace(view_id, idx);

    if (inserted) {
        m_buckets.push_back(CanvasBucket{ view_id, {} });
        m_current_idx = idx;
    } else {
        m_current_idx = it->second;
    }

    m_stack.push_back(view_id);
}

void Canvas::popView() {
    m_current_idx = kInvalidIdx;
    DEV_ASSERT(!m_stack.empty());
    if (m_stack.empty()) {
        return;
    }

    m_stack.pop_back();
    if (m_stack.empty()) {
        return;
    }

    auto it = m_lookup.find(m_stack.back());
    if (it == m_lookup.end()) {
        CRASH_NOW_MSG("invalid view stack");
        return;
    }

    m_current_idx = it->second;
}

bool Canvas::takeBucket(ViewId view_id, CanvasBucket& out) {
    DEV_ASSERT(!m_can_submit);
    auto it = m_lookup.find(view_id);
    if (it == m_lookup.end()) {
        return false;
    }
    if (DEV_VERIFY(it->second < static_cast<int>(m_buckets.size()))) {
        out = std::move(m_buckets[it->second]);
        return true;
    }
    return false;
}

void Canvas::addImage(GpuTexture* texture,
                      const math::Vec2f& min,
                      const math::Vec2f& max,
                      const math::Vec2f& uv_min,
                      const math::Vec2f& uv_max,
                      const math::Vec4f& tint,
                      const math::Mat4f* transform) {
    DEV_ASSERT(texture);
    if (canSubmit()) {
        addImageImpl(texture, min, max, uv_min, uv_max, tint, transform);
    }
}

void Canvas::addBox2(const Vec2f& min,
                     const Vec2f& max,
                     const Vec4f& tint,
                     const Mat4f* transform) {
    if (canSubmit()) {
        addImageImpl(nullptr, min, max, Vec2f::Zero, Vec2f::Zero, tint, transform);
    }
}

void Canvas::addBox2Frame(const Vec2f& min,
                          const Vec2f& max,
                          float thickness,
                          const Vec4f& tint,
                          const Mat4f* transform) {
    if (!canSubmit()) {
        return;
    }

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

void Canvas::addImageImpl(GpuTexture* texture,
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

    m_buckets[m_current_idx].shapes.push_back(shape);
}

}  // namespace cave
