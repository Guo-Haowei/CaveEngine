#include "example_base.h"
#include "loader.h"

#include "engine/math/matrix_transform.h"

#ifndef ASSET_DIR
#define ASSET_DIR
#endif

using namespace cave;
using namespace rs;

class TextureVs : public IVertexShader {
   public:
    TextureVs()
        : IVertexShader(sVaryingFlags) {}

    virtual VSOutput processVertex(const VSInput &input) override {
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
    virtual Color processFragment(const VSOutput &input) override {
        Color color = m_cubeTexture->sample(input.uv);
        return color;
    }

   public:
    const Texture *m_cubeTexture;
};

VSInput g_vertices[4];

const unsigned int g_indices[6] = { 0, 1, 2, 0, 2, 3 };

class TextureTest : public ExampleBase {
   public:
    TextureTest();

    virtual void postInit() override;
    virtual void update(double deltaTime) override;

   private:
    TextureVs m_vs;
    TextureFs m_fs;

    Texture m_texture;

    Matrix4x4f P;
    Matrix4x4f V;
    Matrix4x4f PV;
};

TextureTest::TextureTest()
    : ExampleBase(g_config) {}

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
    const float fovy = 0.785398f;  // 45.0 degree
    V = LookAtRh(Vector3f(0, -2, 2), Vector3f(0), Vector3f(0, 1, 0));
    P = BuildPerspectiveRH(fovy, (float)m_width / (float)m_height, 0.1f, 10.0f);
    PV = P * V;

    // texture
    loadTexture(m_texture, SOURCE_DIR "/texture.jpg");
    m_fs.m_cubeTexture = &m_texture;
}

void TextureTest::update(double deltaTime) {
    static const Matrix4x4f M0 = Translate(Vector3f(0.0f, 0.0f, 0.5f));
    static const Matrix4x4f M1 = Translate(Vector3f(0.0f, 0.0f, -0.5f));
    rs::clear(COLOR_DEPTH_BUFFER_BIT);
    m_vs.PVM = PV * M0;
    rs::drawElements(0, 6);
    m_vs.PVM = PV * Matrix4x4f(1);
    rs::drawElements(0, 6);
    m_vs.PVM = PV * M1;
    rs::drawElements(0, 6);
}

ExampleBase *g_pExample = new TextureTest();
Config g_config = { 900, 540, "Texture" };
