#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/stb_image_write.h>

#include "engine/assets/image_asset.h"
#include "engine/assets/material_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/math/matrix_transform.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"

#include "modules/sw/pbr_pipeline.h"
#include "modules/sw/sw_renderer.h"

#include "thumbnail.h"

namespace cave {

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
        auto all_materials = m_asset_registry->GetAssetsOfType(AssetType::Material);

        // @TODO: proper setup
        m_pipeline.per_batch_cb.c_worldMatrix = Rotate(Degree(-30.0f), Vector3f::UnitY);
        m_pipeline.per_frame_cb.c_cameraPosition = CAM_POS;
        m_pipeline.per_frame_cb.c_camView = LookAtRh(CAM_POS, Vector3f::Zero, Vector3f::UnitY);
        m_pipeline.per_frame_cb.c_camProj = BuildOpenGlPerspectiveRH(Degree(45.0f).GetRadians(), 1.0f, 0.1f, 100.0f);

        thumbnail::FillDefaultMaterial(m_pipeline.material_cb);

        for (const auto& entry : all_meshes) {
            const MeshAsset* mesh = entry.Get<MeshAsset>();
            mesh->gpuResource = m_graphics_manager->CreateMesh(*mesh).value_or(nullptr);

            thumbnail::DrawMesh(mesh->gpuResource.get(), *sw);

            std::vector<Color> color = thumbnail::Convert(m_render_target.m_colorBuffer.m_buffer);

            std::string path = std::format("@res://_cache/{}@{}x{}.png", entry.GetGuid().ToString(), m_dim, m_dim);
            path = FileAccess::FixPath(FileAccess::ACCESS_RESOURCE, path);
            stbi_write_png(path.c_str(), m_dim, m_dim, 4, color.data(), m_dim * sizeof(Color));
        }

        auto sphere = AssetRegistry::GetSingleton().FindByPath<MeshAsset>("@persist://meshes/sphere").unwrap().Get();

        for (const auto& entry : all_materials) {
            const MaterialAsset* mat = entry.Get<MaterialAsset>();
            m_pipeline.material_cb.c_baseColor = mat->base_color;
            m_pipeline.material_cb.c_metallic = mat->metallic;
            m_pipeline.material_cb.c_roughness = mat->roughness;
            m_pipeline.material_cb.c_emissivePower = mat->emissive;
            m_pipeline.material_cb.c_hasBaseColorMap = false;
            auto handle = AssetRegistry::GetSingleton().FindByGuid<ImageAsset>(mat->textures[std::to_underlying(TextureSlot::Base)]);
            if (handle.is_some()) {
                const ImageAsset* image = handle.unwrap_unchecked().Get();
                if (image) {
                    m_pipeline.material_cb.c_hasBaseColorMap = true;
                    m_pipeline.material_cb.c_baseColorMapHandle = (size_t)image;
                }
            }

            thumbnail::DrawMesh(sphere->gpuResource.get(), *sw);
            std::vector<Color> color = thumbnail::Convert(m_render_target.m_colorBuffer.m_buffer);
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
