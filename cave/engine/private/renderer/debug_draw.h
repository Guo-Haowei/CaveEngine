#pragma once
#include "engine/private/core/math/geomath.h"

namespace cave {

struct GpuTexture;
struct GpuMesh;

class DebugDrawService {
    struct Item {
        math::Vec3f min;
        math::Vec3f max;
        math::Vec4f tint_color;
        GpuTexture* texture = nullptr;
    };

public:
    void addBox2Frame(const math::Vec2f& min,
                      const math::Vec2f& max,
                      const math::Vec4f& color,
                      const math::Mat4f* transform = nullptr,
                      float thickness = 0.1f);

    void addBox2(const math::Vec2f& min,
                 const math::Vec2f& max,
                 const math::Vec4f& color,
                 const math::Mat4f* transform = nullptr);

    void batch();

    const GpuMesh* gpuMesh() const { return mesh_.get(); }

private:
    std::shared_ptr<GpuMesh> mesh_;
    std::vector<Item> items_;
};

}  // namespace cave
