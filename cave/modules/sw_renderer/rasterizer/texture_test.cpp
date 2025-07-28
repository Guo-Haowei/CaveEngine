#include "example_base.h"

#include "engine/math/matrix_transform.h"
#define STB_IMAGE_IMPLEMENTATION
#include "tinygltf/stb_image.h"

#ifndef ASSET_DIR
#define ASSET_DIR
#endif

namespace cave {

using namespace rs;

// @TODO: get rid of this function,
// use asset system instead
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

class TextureTest : public ExampleBase {
public:
    TextureTest();

    virtual void postInit() override;
    virtual void update() override;

private:
    TextureVs m_vs;
    TextureFs m_fs;

    Texture m_texture;

    Matrix4x4f P;
    Matrix4x4f V;
    Matrix4x4f PV;
};

TextureTest::TextureTest()
    : ExampleBase() {
    m_width = m_height = 256;
}

void TextureTest::postInit() {
    rs::setSize(m_width, m_height);
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
    constexpr float w = .5f;
    V = LookAtRh(Vector3f(0, 0, 4), Vector3f::Zero, Vector3f::UnitY);
    P = BuildOpenGlOrthoRH(-w, w, -w, w, 1.0f, 100.0f);
    PV = P * V;

    // texture
    loadTexture(m_texture, SOURCE_DIR "/texture.jpg");
    m_fs.m_cubeTexture = &m_texture;
}

void TextureTest::update() {
    rs::clear(COLOR_DEPTH_BUFFER_BIT);
    m_vs.PVM = PV;
    rs::drawElements(0, 6);
}

ExampleBase* g_pExample = new TextureTest();
}  // namespace cave
