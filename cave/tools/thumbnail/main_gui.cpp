#include "engine/assets/image_asset.h"
#include "engine/assets/mesh_asset.h"
#include "engine/drivers/windows/win32_display_manager.h"
#include "engine/math/matrix_transform.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"

#include "modules/sw/pbr_pipeline.h"
#include "modules/sw/sw_renderer.h"

#include "thumbnail.h"

namespace cave {

static constexpr Vector3f CAM_POS{ 0.0f, 1.f, 2.3f };

class FpsCounter {
public:
    void Frame() {
        ++m_frameCount;
        auto now = Clock::now();
        std::chrono::duration<float> delta = now - m_lastTime;

        if (delta.count() >= 1.0f) {
            m_fps = m_frameCount / delta.count();
            m_frameCount = 0;
            m_lastTime = now;
        }
    }

    float GetFPS() const {
        return m_fps;
    }

    std::string GetFPSString(int precision = 1) const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << m_fps << " FPS";
        return oss.str();
    }

private:
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point m_lastTime = Clock::now();
    int m_frameCount = 0;
    float m_fps = 0.0f;
};

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
        m_render_target.create({ m_dim, m_dim, true, true });
        sw->setRenderTarget(&m_render_target);

        sw->setSize(m_dim, m_dim);
        sw->SetPipeline(&m_pipeline);

        // model
        m_mesh = AssetRegistry::GetSingleton().FindByPath<MeshAsset>("@persist://meshes/sphere").unwrap().Get();
        m_mesh->gpuResource = m_graphics_manager->CreateMesh(*m_mesh).value_or(nullptr);

#if 0
        // texture
        ImageAsset* image = AssetRegistry::GetSingleton().FindByPath<ImageAsset>("@res://images/uv.png").unwrap().Get();
        DEV_ASSERT(image);

        m_texture.create({ image->width, image->height, image->buffer.data() });
        m_pipeline.m_texture = &m_texture;
#endif

        // @TODO: proper setup
        m_pipeline.per_batch_cb.c_worldMatrix = Rotate(Degree(30.0f), Vector3f::UnitY);
        m_pipeline.per_frame_cb.c_cameraPosition = CAM_POS;
        m_pipeline.per_frame_cb.c_camView = LookAtRh(CAM_POS, Vector3f::Zero, Vector3f::UnitY);
        m_pipeline.per_frame_cb.c_camProj = BuildOpenGlPerspectiveRH(Degree(45.0f).GetRadians(), 1.0f, 0.1f, 100.0f);

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

        m_fps_counter.Frame();

        thumbnail::FillDefaultMaterial(m_pipeline.material_cb);

        thumbnail::DrawMesh(m_mesh->gpuResource.get(), *m_graphics_manager);

        std::vector<Color> color = thumbnail::Convert(m_render_target.m_colorBuffer.m_buffer, true);
        DrawPixels(color.data());

        std::string title = std::format("SwRenderer (fps: {})", m_fps_counter.GetFPS());
        m_display_server->SetTitle(title);

        return true;
    }

    SwRenderTarget m_render_target;
    PbrPipeline m_pipeline;

    MeshAsset* m_mesh;
    int m_dim;

    FpsCounter m_fps_counter;

    BITMAP m_bitmap;
    HBITMAP m_map;
    HDC m_src;
    HDC m_hdc = NULL;
};

Application* CreateGuiApp(const ApplicationSpec& p_spec) {
    return new GuiApp(p_spec);
}

}  // namespace cave
