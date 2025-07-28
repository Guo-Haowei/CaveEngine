#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/stb_image_write.h>

#include "engine/assets/image_asset.h"
#include "engine/assets/mesh_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/math/matrix_transform.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"

#include "modules/sw/pbr_pipeline.h"
#include "modules/sw/sw_renderer.h"

#include "thumbnail_dvars.h"

namespace cave {

static constexpr Vector3f CAM_POS{ 0.0f, 1.f, 2.3f };

class CliApp : public Application {
public:
    CliApp(const ApplicationSpec& p_spec)
        : Application(p_spec, Application::Type::Tool) {
    }

    CameraComponent* GetActiveCamera() override {
        return nullptr;
    }

    bool IsWorld2D() const override {
        return false;
    }

    Result<void> Initialize() override {
        if (auto res = Application::Initialize(); !res) {
            return CAVE_ERROR(res.error());
        }

        m_dim = DVAR_GET_INT(thumbnail_size);

        auto sw = static_cast<SwGraphicsManager*>(m_graphics_manager);

        // @TODO: refactor
        m_render_target.create({ m_dim, m_dim, true, true });
        sw->setRenderTarget(&m_render_target);

        sw->setSize(m_dim, m_dim);
        sw->SetPipeline(&m_pipeline);

        auto all_meshes = m_asset_registry->GetAssetsOfType(AssetType::Mesh);

        // @TODO: proper setup
        m_pipeline.per_batch_cb.c_worldMatrix = Rotate(Degree(30.0f), Vector3f::UnitY);
        m_pipeline.per_frame_cb.c_cameraPosition = CAM_POS;
        m_pipeline.per_frame_cb.c_camView = LookAtRh(CAM_POS, Vector3f::Zero, Vector3f::UnitY);
        m_pipeline.per_frame_cb.c_camProj = BuildOpenGlPerspectiveRH(Degree(45.0f).GetRadians(), 1.0f, 0.1f, 100.0f);

        for (const auto& entry : all_meshes) {
            auto mesh = m_graphics_manager->CreateMesh(*entry.Get<MeshAsset>()).value_or(nullptr);
            sw->SetMesh(mesh.get());

            const auto clear_flag = ClearFlags::CLEAR_COLOR_BIT | ClearFlags::CLEAR_DEPTH_BIT;

            sw->Clear(nullptr, clear_flag, &AMBIENT_COLOR.r);
            sw->DrawElements(mesh->desc.drawCount);

            auto convert = [](float v) {
                return static_cast<uint8_t>(clamp(255.f * v, 0.0f, 255.f));
            };

            const auto& buffer = m_render_target.m_colorBuffer.m_buffer;
            std::vector<Color> color(buffer.size());
            constexpr float gamma = 1.0f / 2.2f;
            for (size_t i = 0; i < buffer.size(); ++i) {
                Vector4f in = buffer[i];
                in = in / (in + 1.0f);
                in.r = glm::pow(in.r, gamma);
                in.g = glm::pow(in.g, gamma);
                in.b = glm::pow(in.b, gamma);

                auto& out = color[i];
                out.r = convert(in.b);
                out.g = convert(in.g);
                out.b = convert(in.r);
                out.a = 255;
            }

            std::string path = std::format("@res://_cache/{}@{}x{}.png", entry.GetGuid().ToString(), m_dim, m_dim);
            path = FileAccess::FixPath(FileAccess::ACCESS_RESOURCE, path);
            stbi_write_png(path.c_str(), m_dim, m_dim, 4, color.data(), m_dim * sizeof(Color));
        }

        return Result<void>();
    }

    void Finalize() {
        Application::Finalize();
    }

protected:
    bool MainLoop() override {
        return false;
    }

    SwRenderTarget m_render_target;
    PbrPipeline m_pipeline;

    int m_dim;
};

Application* CreateCliApp(const ApplicationSpec& p_spec) {
    return new CliApp(p_spec);
}

}  // namespace cave
