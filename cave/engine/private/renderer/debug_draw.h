#pragma once
#include "engine/private/core/math/geomath.h"

namespace cave {

struct GpuTexture;
struct GpuMesh;

class DebugDraw {
    struct Item {
        math::Vec3f min;
        math::Vec3f max;
        math::Vec4f tint_color;
        GpuTexture* texture = nullptr;
    };

public:
    void AddBox2Frame(const math::Vec2f& p_min,
                      const math::Vec2f& p_max,
                      const math::Vec4f& p_color,
                      const math::Matrix4x4f* p_transform = nullptr,
                      float p_thickness = 0.1f);

    void AddBox2(const math::Vec2f& p_min,
                 const math::Vec2f& p_max,
                 const math::Vec4f& p_color,
                 const math::Matrix4x4f* p_transform = nullptr);

    void Batch();

    const GpuMesh* GetGpuMesh() const { return m_mesh.get(); }

private:
    std::shared_ptr<GpuMesh> m_mesh;
    std::vector<Item> m_items;
};

}  // namespace cave
