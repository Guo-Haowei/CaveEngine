#include "engine/assets/image_asset.h"
#include "engine/assets/mesh_asset.h"
#include "engine/core/string/string_utils.h"
#include "engine/drivers/windows/win32_display_manager.h"
#include "engine/empty/empty_graphics_manager.h"
#include "engine/math/matrix_transform.h"
#include "engine/math/vector.h"
#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/entry_point.h"
#include "engine/runtime/mode_manager.h"
#include "modules/sw/sw_renderer.h"

#include "pbr.hlsl.h"

namespace cave {

constexpr int DIM = 256;
namespace fs = std::filesystem;
using namespace rs;

//---------------

constexpr Vector3f CAM_POS{ 0, 0, 2 };
constexpr Vector3f LIGHT_POS{ 0, 4, 4 };
constexpr Vector4f AMBIENT_COLOR{ 0.04f, 0.04f, 0.04f, 1.0f };

class PbrPipeline : public SwPipeline {
public:
    PbrPipeline()
        : SwPipeline(VARYING_UV | VARYING_NORMAL | VARYING_WORLD_POSITION) {}

    virtual VSOutput ProcessVertex(const VSInput& input) override {
        VSOutput vs_output;
        vs_output.world_position = M * input.position;
        vs_output.position = PV * vs_output.world_position;
        vs_output.normal = M * input.normal;
        vs_output.uv = input.uv;
        return vs_output;
    }

    // @TODO: move to sample
    FORCE_INLINE float srgb_to_linear(float s) {
        return std::pow((s + 0.055f) / 1.055f, 2.4f);
    }

    virtual Vector3f ProcessFragment(const VSOutput& input) override {
        // @TODO:
#if 1
        Color color;
        color = m_texture->sample(input.uv);
        Vector3f base_color;
        base_color.r = srgb_to_linear(color.r / 255.f);
        base_color.g = srgb_to_linear(color.g / 255.f);
        base_color.b = srgb_to_linear(color.b / 255.f);
#else
        Vector3f base_color = Vector3f::UnitX;
#endif

        Vector3f final_color = ComputeLighting(base_color,
                                               Vector3f::Zero,
                                               input.normal.xyz,
                                               0.4f,
                                               0.6f,
                                               0.0f);

        return final_color;
    }

    Vector3f ComputeLighting(Vector3f base_color,
                             Vector3f world_position,
                             Vector3f N,
                             float metallic,
                             float roughness,
                             float emissive) {
        if (emissive > 0.0) {
            return Vector3f(emissive * base_color);
        }

        const Vector3f V = normalize(c_cameraPosition - world_position);
        const float NdotV = cave::max(dot(N, V), 0.0f);
        Vector3f R = glm::reflect(-V, N);

        Vector3f Lo = Vector3f(0.0f);
        Vector3f F0 = cave::lerp(Vector3f(0.04f), base_color, metallic);

        const Vector3f radiance = Vector3f(4.0f);
        Vector3f delta = -world_position + LIGHT_POS;
        Vector3f L = normalize(delta);
        const Vector3f H = normalize(V + L);
        Vector3f direct_lighting = lighting(N, L, V, radiance, F0, roughness, metallic, base_color);
        Lo += direct_lighting;

        Vector3f F = FresnelSchlickRoughness(NdotV, F0, roughness);
        Vector3f kS = F;
        Vector3f kD = 1.0f - kS;
        kD *= 1.0f - metallic;

#if 1
        Vector3f diffuse = AMBIENT_COLOR.xyz;
        Vector3f specular = Vector3f(0.0f);

        const float ao = 1.0;
        Vector3f ambient = (kD * diffuse + specular) * ao;
#endif

        Vector3f final_color = Lo + ambient;
        return final_color;
    }

public:
    const Texture* m_texture;
    Vector3f c_cameraPosition;

    Matrix4x4f M;
    Matrix4x4f PV;
};

//---------------

void RegisterExtraDvars() {}

class MyApp : public Application {
public:
    MyApp(const ApplicationSpec& p_spec)
        : Application(p_spec, Application::Type::Tool) {
        m_mode_manager = std::unique_ptr<ModeManager>(new ModeManager(GameMode::Tool, *this));
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

        m_map = CreateBitmap(DIM,       // width
                             DIM,       // height
                             1,         // Color Planes
                             32,        // 4 * 8 bits
                             nullptr);  // pointer to array

        m_src = CreateCompatibleDC(m_hdc);
        SelectObject(m_src, m_map);

        auto sw = static_cast<SwGraphicsManager*>(m_graphics_manager);

        // @TODO: refactor
        m_renderTarget.create({ DIM, DIM, true, true });
        sw->setRenderTarget(&m_renderTarget);

        sw->setSize(DIM, DIM);
        sw->SetPipeline(&m_pipeline);

        // model
#if 1
        //m_mesh = AssetRegistry::GetSingleton().FindByPath<MeshAsset>("@persist://meshes/sphere").unwrap().Get();
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
        SetBitmapBits(m_map, DIM * DIM * 4, p_data);

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

    rs::RenderTarget m_renderTarget;
    PbrPipeline m_pipeline;

    MeshAsset* m_mesh;

    Texture m_texture;

    Matrix4x4f P;
    Matrix4x4f V;
    Matrix4x4f PV;

    BITMAP m_bitmap;
    HBITMAP m_map;
    HDC m_src;
    HDC m_hdc = NULL;
};

Application* CreateApplication() {
    // @TODO: get rid of this
    std::string_view root = StringUtils::BasePath(__FILE__);
    root = StringUtils::BasePath(root);
    root = StringUtils::BasePath(root);

    auto user_path = fs::path{ root } / "user";
    auto user_string = user_path.string();

    ApplicationSpec spec{};
    spec.userFolder = user_string;
    spec.name = "SoftwareRenderer";
    spec.width = DIM;
    spec.height = DIM;
    spec.backend = Backend::EMPTY;
    spec.decorated = true;
    spec.fullscreen = false;
    spec.vsync = false;
    spec.enableImgui = false;
    return new MyApp(spec);
}

}  // namespace cave

int main(int p_argc, const char** p_argv) {
    using namespace cave;

    IDisplayManager::RegisterCreateFunc([]() -> IDisplayManager* {
        return new Win32DisplayManager();
    });
    IGraphicsManager::RegisterCreateFunc([]() -> IGraphicsManager* {
        return new SwGraphicsManager();
    });

    return Main(p_argc, p_argv);
}
