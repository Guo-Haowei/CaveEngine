// =============================================================================
// File: cave/runtime/display/ICanvas.h
// =============================================================================
#pragma once
#include <span>

#include "cave/core/math/Matrix.h"
#include "cave/core/ids/ViewId.h"

#include "cave/render/PrimitiveData.h"

namespace cave {

struct GpuTexture;

enum class PrimShapeType : uint8_t {
    Line = 0,
    Triangle,
    Rect,
    // Circle,
};

struct PrimShape {
    PrimShapeType type;
    std::array<render::PrimVert, 4> vertices;
    GpuTexture* tex{};
};

struct CanvasBucket {
    ViewId view_id;
    Vector<PrimShape> shapes;
};

class ICanvas {
public:
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    virtual void pushView(ViewId view_id) = 0;
    virtual void popView() = 0;

    virtual void addBox2(const math::Vec2f& min,
                         const math::Vec2f& max,
                         const math::Vec4f& tint = math::Vec4f::One,
                         const math::Mat4f* transform = nullptr) = 0;

    virtual void addBox2Frame(const math::Vec2f& min,
                              const math::Vec2f& max,
                              float thickness,
                              const math::Vec4f& tint = math::Vec4f::One,
                              const math::Mat4f* transform = nullptr) = 0;

    virtual void addImage(GpuTexture* texture,
                          const math::Vec2f& min,
                          const math::Vec2f& max,
                          const math::Vec2f& uv_min = math::Vec2f::Zero,
                          const math::Vec2f& uv_max = math::Vec2f::One,
                          const math::Vec4f& tint = math::Vec4f::One,
                          const math::Mat4f* transform = nullptr) = 0;

    virtual bool takeBucket(ViewId view_id, CanvasBucket& out) = 0;
};

}  // namespace cave