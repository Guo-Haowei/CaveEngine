#pragma once
#include "engine/math/geomath.h"

namespace cave {

struct GpuTexture;
struct GpuMesh;

class DebugDraw {
    struct Item {
        Vector3f min;
        Vector3f max;
        Vector4f tint_color;
        GpuTexture* texture = nullptr;
    };

public:
    void AddBox2(const Vector2f& p_center,
                 const Vector2f& p_half,
                 const Vector4f& p_color,
                 const Matrix4x4f* p_transform = nullptr);

    void Batch();

    const GpuMesh* GetGpuMesh() const { return m_mesh.get(); }

private:
    std::shared_ptr<GpuMesh> m_mesh;
    std::vector<Item> m_items;
};

}  // namespace cave
