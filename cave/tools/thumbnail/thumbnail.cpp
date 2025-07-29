#include "thumbnail.h"

namespace cave::thumbnail {

FORCE_INLINE uint8_t ConvertChannel(float v) {
    return static_cast<uint8_t>(clamp(255.f * v, 0.0f, 255.f));
};

std::vector<Color> Convert(const std::vector<Vector4f>& p_buffer, bool p_to_bgra) {
    std::vector<Color> color(p_buffer.size());
    constexpr float gamma = 1.0f / 2.2f;
    for (size_t i = 0; i < p_buffer.size(); ++i) {
        Vector4f in = p_buffer[i];
        in = in / (in + 1.0f);
        in.r = glm::pow(in.r, gamma);
        in.g = glm::pow(in.g, gamma);
        in.b = glm::pow(in.b, gamma);

        auto& out = color[i];
        out.r = ConvertChannel(in.r);
        out.g = ConvertChannel(in.g);
        out.b = ConvertChannel(in.b);
        if (p_to_bgra) {
            std::swap(out.r, out.b);
        }
        out.a = 255;
    }

    return color;
}

void DrawMesh(const GpuMesh* p_mesh, IGraphicsManager& p_graphics_manager) {
    p_graphics_manager.SetMesh(p_mesh);
    ClearFlags clear_flag = ClearFlags::CLEAR_COLOR_BIT | ClearFlags::CLEAR_DEPTH_BIT;
    p_graphics_manager.Clear(nullptr, clear_flag, &AMBIENT_COLOR.r);
    p_graphics_manager.DrawElements(p_mesh->desc.drawCount);
}

void FillDefaultMaterial(MaterialConstantBuffer& p_out) {
    p_out.c_baseColor = Vector4f(0.5f, 0.5f, 0.5f, 1.0f);
    p_out.c_metallic = 0.2f;
    p_out.c_roughness = 0.8f;
    p_out.c_emissivePower = 0.0f;
    p_out.c_hasBaseColorMap = false;
}

}  // namespace cave::thumbnail
