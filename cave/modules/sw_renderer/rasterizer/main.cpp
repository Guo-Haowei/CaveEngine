#include "engine/core/string/string_utils.h"
#include "engine/drivers/windows/win32_display_manager.h"
#include "engine/math/matrix_transform.h"
#include "engine/empty/empty_graphics_manager.h"
#include "engine/runtime/application.h"
#include "engine/runtime/entry_point.h"
#include "engine/runtime/mode_manager.h"

//---------------
#include "rasterizer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "tinygltf/stb_image.h"
//---------------

namespace cave {

constexpr int DIM = 256;
namespace fs = std::filesystem;
using namespace rs;

//---------------
// @TODO: refactor this part
static void loadTexture(Texture& texture, const char* path) {
    int width, height, channel;
    unsigned char* data = stbi_load(path, &width, &height, &channel, 0);
    if (data) {
        std::vector<Color> buffer(width * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int bufferIndex = y * width + x;
                Color& c = buffer[bufferIndex];
                int channelIndex = channel * bufferIndex;
                // NOTE: bitmap is in bgra format
                c.r = data[channelIndex + 2];
                c.g = data[channelIndex + 1];
                c.b = data[channelIndex + 0];
                c.a = channel == 4 ? data[channelIndex + 3] : 255;
            }
        }

        texture.create({ width, height, buffer.data() });
        stbi_image_free(data);
    } else {
        printf("Failed to load texture '%s'\n", path);
    }
}

class TextureVs : public IVertexShader {
public:
    TextureVs()
        : IVertexShader(sVaryingFlags) {}

    virtual VSOutput processVertex(const VSInput& input) override {
        VSOutput vs_output;
        vs_output.position = input.position;
        vs_output.uv = input.position.xy;
        vs_output.uv += 0.5f;
        vs_output.uv.y = 1.0f - vs_output.uv.y;
        vs_output.position = PVM * vs_output.position;
        return vs_output;
    }

public:
    Matrix4x4f PVM;

private:
    static const unsigned int sVaryingFlags = VARYING_UV;
};

class TextureFs : public IFragmentShader {
public:
    virtual Color processFragment(const VSOutput& input) override {
        Color color = m_cubeTexture->sample(input.uv);
        return color;
    }

public:
    const Texture* m_cubeTexture;
};

VSInput g_vertices[4];

const unsigned int g_indices[6] = { 0, 1, 2, 0, 2, 3 };

//---------------

void RegisterExtraDvars() {}

class SwRendererApp : public Application {
public:
    SwRendererApp(const ApplicationSpec& p_spec)
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

        // @TODO: refactor
        m_renderTarget.create({ DIM, DIM, true, true });
        rs::setRenderTarget(&m_renderTarget);
        rs::setSize(DIM, DIM);

        rs::setSize(DIM, DIM);
        rs::setVertexShader(&m_vs);
        rs::setFragmentShader(&m_fs);

        // mesh
        g_vertices[0].position = Vector4f{ -0.5f, +0.5f, 0.0f, 1.0f };  // top left
        g_vertices[1].position = Vector4f{ -0.5f, -0.5f, 0.0f, 1.0f };  // bottom left
        g_vertices[2].position = Vector4f{ +0.5f, -0.5f, 0.0f, 1.0f };  // bottom right
        g_vertices[3].position = Vector4f{ +0.5f, +0.5f, 0.0f, 1.0f };  // top right

        rs::setVertexArray(g_vertices);
        rs::setIndexArray(g_indices);

        // constant buffer
        constexpr float w = 1.0f;
        V = LookAtRh(Vector3f(0, 0, 4), Vector3f::Zero, Vector3f::UnitY);
        P = BuildOpenGlOrthoRH(-w, w, -w, w, 1.0f, 100.0f);
        PV = P * V;

        // texture
        loadTexture(m_texture, SOURCE_DIR "/texture.jpg");
        m_fs.m_cubeTexture = &m_texture;

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

        rs::clear(COLOR_DEPTH_BUFFER_BIT);
        m_vs.PVM = PV;
        rs::drawElements(0, 6);

        DrawPixels(m_renderTarget.getColorBuffer().getData());

        return true;
    }

    rs::RenderTarget m_renderTarget;
    TextureVs m_vs;
    TextureFs m_fs;

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
    spec.width = 256;
    spec.height = 256;
    spec.backend = Backend::EMPTY;
    spec.decorated = true;
    spec.fullscreen = false;
    spec.vsync = false;
    spec.enableImgui = false;
    return new SwRendererApp(spec);
}

}  // namespace cave

int main(int p_argc, const char** p_argv) {
    using namespace cave;

    IDisplayManager::RegisterCreateFunc([]() -> IDisplayManager* {
        return new Win32DisplayManager();
    });
    IGraphicsManager::RegisterCreateFunc([]() -> IGraphicsManager* {
        return new EmptyGraphicsManager();
    });

    return Main(p_argc, p_argv);
}
