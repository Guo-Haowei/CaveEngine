// =============================================================================
// File: cave/runtime/display/IDebugDrawService.h
// =============================================================================
#pragma once
#include <span>

#include "cave/core/math/Matrix.h"

namespace cave {

struct GpuTexture;

struct DebugDrawItem {
    math::Vec3f min;
    math::Vec3f max;
    math::Vec4f tint_color;
    GpuTexture* texture = nullptr;
};

class IDebugDrawService {
public:
    virtual void addBox2Frame(const math::Vec2f& min,
                              const math::Vec2f& max,
                              const math::Vec4f& color,
                              float thickness = 0.1f,
                              const math::Mat4f* transform = nullptr) = 0;

    virtual void addBox2(const math::Vec2f& min,
                         const math::Vec2f& max,
                         const math::Vec4f& color,
                         const math::Mat4f* transform = nullptr) = 0;

    virtual auto items() const -> std::span<const DebugDrawItem> = 0;
    virtual void clear() = 0;
};

}  // namespace cave