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
    void AddRect(const Vector3f& p_center,
                 const Vector3f& p_half,
                 const Vector4f& p_color,
                 const Matrix4x4f* p_transform = nullptr);

    void Batch();

private:
    std::shared_ptr<GpuMesh> m_mesh;
    std::vector<Item> m_items;
};

}  // namespace cave
