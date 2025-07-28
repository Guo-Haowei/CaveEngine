#include "engine/assets/image_asset.h"
#include "engine/assets/mesh_asset.h"
#include "engine/drivers/windows/win32_display_manager.h"
#include "engine/math/matrix_transform.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"

#include "modules/sw/pbr_pipeline.h"
#include "modules/sw/sw_renderer.h"

#include "thumbnail_dvars.h"

namespace cave {

class GuiApp : public Application {
public:
    GuiApp(const ApplicationSpec& p_spec)
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

        auto& display = static_cast<Win32DisplayManager&>(IDisplayManager::GetSingleton());

        m_hdc = GetDC(display.GetHwnd());

        HGDIOBJ hBitmap = GetCurrentObject(m_hdc, OBJ_BITMAP);
        GetObject(hBitmap, sizeof(BITMAP), &m_bitmap);

        m_dim = DVAR_GET_INT(thumbnail_size);

        m_map = CreateBitmap(m_dim,     // width
                             m_dim,     // height
                             1,         // Color Planes
                             32,        // 4 * 8 bits
                             nullptr);  // pointer to array

        m_src = CreateCompatibleDC(m_hdc);
        SelectObject(m_src, m_map);

        auto sw = static_cast<SwGraphicsManager*>(m_graphics_manager);

        // @TODO: refactor
        m_renderTarget.create({ m_dim, m_dim, true, true });
        sw->setRenderTarget(&m_renderTarget);

        sw->setSize(m_dim, m_dim);
        sw->SetPipeline(&m_pipeline);

        // model
#if 1
        // m_mesh = AssetRegistry::GetSingleton().FindByPath<MeshAsset>("@persist://meshes/sphere").unwrap().Get();
        m_mesh = AssetRegistry::GetSingleton().FindByPath<MeshAsset>("@persist://meshes/torus").unwrap().Get();
#else
        m_mesh = AssetRegistry::GetSingleton().FindByPath<MeshAsset>("@persist://meshes/cube").unwrap().Get();
#endif
        m_mesh->gpuResource = m_graphics_manager->CreateMesh(*m_mesh).value_or(nullptr);

        // texture
        ImageAsset* image = AssetRegistry::GetSingleton().FindByPath<ImageAsset>("@res://images/uv.png").unwrap().Get();
        DEV_ASSERT(image);

        m_texture.create({ image->width, image->height, image->buffer.data() });
        m_pipeline.m_texture = &m_texture;

        // constant buffer
        constexpr float w = 1.0f;

        m_pipeline.c_cameraPosition = CAM_POS;
        V = LookAtRh(CAM_POS, Vector3f::Zero, Vector3f::UnitY);
        P = BuildOpenGlOrthoRH(-w, w, -w, w, 1.0f, 100.0f);
        P = BuildOpenGlPerspectiveRH(Degree(45.0f).GetRadians(), 1.0f, 0.1f, 100.0f);
        PV = P * V;

        return Result<void>();
    }

    void Finalize() {
        DeleteObject(m_map);
        DeleteDC(m_src);

        Application::Finalize();
    }

    void DrawPixels(const void* p_data) {
        SetBitmapBits(m_map, m_dim * m_dim * 4, p_data);

        BitBlt(m_hdc,              // Destination
               0,                  // x and
               0,                  // y - upper-left corner of place, where we'd like to copy
               m_bitmap.bmWidth,   // width of the region
               m_bitmap.bmHeight,  // height
               m_src,              // source
               0,                  // x and
               0,                  // y of upper left corner  of part of the source, from where we'd like to copy
               SRCCOPY);           // Defined DWORD to juct copy pixels. Watch more on msdn;
    }

protected:
    bool MainLoop() override {
        m_display_server->BeginFrame();
        if (m_display_server->ShouldClose()) {
            return false;
        }

        auto& sw = m_graphics_manager;

        sw->SetMesh(m_mesh->gpuResource.get());

        m_pipeline.M = Rotate(Degree(45.0f), Vector3f::UnitX);
        m_pipeline.PV = PV;

        // @TODO: viewport

        const auto clear_flag = ClearFlags::CLEAR_COLOR_BIT | ClearFlags::CLEAR_DEPTH_BIT;
        // @TODO: render target

        sw->Clear(nullptr, clear_flag, &AMBIENT_COLOR.r);
        sw->DrawElements(m_mesh->gpuResource->desc.drawCount);

        auto convert = [](float v) {
            return static_cast<uint8_t>(clamp(255.f * v, 0.0f, 255.f));
        };

        // @TODO: gamma correct
        const auto& buffer = m_renderTarget.m_colorBuffer.m_buffer;
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

        DrawPixels(color.data());

        return true;
    }

    // @TODO: refactor
    SwTexture<Color> m_texture;

    Matrix4x4f P;
    Matrix4x4f V;
    Matrix4x4f PV;
    /// </summary>

    SwRenderTarget m_renderTarget;
    PbrPipeline m_pipeline;

    MeshAsset* m_mesh;
    int m_dim;

    BITMAP m_bitmap;
    HBITMAP m_map;
    HDC m_src;
    HDC m_hdc = NULL;
};

Application* CreateGuiApp(const ApplicationSpec& p_spec) {
    return new GuiApp(p_spec);
}

}  // namespace cave
