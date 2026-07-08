#pragma once
#include <memory>

#include "cave/runtime/display/IDebugDrawService.h"

namespace cave {

class Canvas : public ICanvas {
public:
    void addBox2Frame(const math::Vec2f& min,
                      const math::Vec2f& max,
                      float thickness,
                      const math::Vec4f& tint,
                      const math::Mat4f* transform) override;

    void addBox2(const math::Vec2f& min,
                 const math::Vec2f& max,
                 const math::Vec4f& tint,
                 const math::Mat4f* transform) override;

    void addImage(GpuTexture* texture,
                  const math::Vec2f& min,
                  const math::Vec2f& max,
                  const math::Vec2f& uv_min,
                  const math::Vec2f& uv_max,
                  const math::Vec4f& tint,
                  const math::Mat4f* transform) override;

    std::span<const PrimShape> primitives() const override {
        return m_shapes;
    }

    void clear() override { m_shapes.clear(); }

private:
    Vector<PrimShape> m_shapes;
};

}  // namespace cave
