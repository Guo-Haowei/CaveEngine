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
    const GpuTexture* tex{};
    int z_index = 0;
};

struct CanvasBucket {
    ViewId view_id;
    Vector<PrimShape> shapes;
};

struct Draw2DOptions {
    int z_index = 0;
    math::Vec4f tint = math::Vec4f::One;
    const math::Mat4f* transform = nullptr;
};

struct ImageDrawOptions : public Draw2DOptions {
    math::Vec2f uv_min = math::Vec2f::Zero;
    math::Vec2f uv_max = math::Vec2f::One;
};

class ICanvas {
public:
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    virtual void pushView(ViewId view_id) = 0;
    virtual void popView() = 0;

    virtual void addBox2(const math::Vec2f& min,
                         const math::Vec2f& max,
                         const Draw2DOptions& options = {}) = 0;

    virtual void addBox2Frame(const math::Vec2f& min,
                              const math::Vec2f& max,
                              float thickness,
                              const Draw2DOptions& options = {}) = 0;

    virtual void addImage(const GpuTexture* texture,
                          const math::Vec2f& min,
                          const math::Vec2f& max,
                          const ImageDrawOptions& options = {}) = 0;

    virtual bool takeBucket(ViewId view_id, CanvasBucket& out) = 0;
};

}  // namespace cave