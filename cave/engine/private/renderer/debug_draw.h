#pragma once
#include "engine/private/math/geomath.h"

namespace cave {

struct GpuTexture;
struct GpuMesh;

class DebugDraw {
    struct Item {
        math::Vector3f min;
        math::Vector3f max;
        math::Vector4f tint_color;
        GpuTexture* texture = nullptr;
    };

public:
    void AddBox2Frame(const math::Vector2f& p_min,
                      const math::Vector2f& p_max,
                      const math::Vector4f& p_color,
                      const math::Matrix4x4f* p_transform = nullptr,
                      float p_thickness = 0.1f);

    void AddBox2(const math::Vector2f& p_min,
                 const math::Vector2f& p_max,
                 const math::Vector4f& p_color,
                 const math::Matrix4x4f* p_transform = nullptr);

    void Batch();

    const GpuMesh* GetGpuMesh() const { return m_mesh.get(); }

private:
    std::shared_ptr<GpuMesh> m_mesh;
    std::vector<Item> m_items;
};

}  // namespace cave
