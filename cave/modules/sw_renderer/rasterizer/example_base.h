#pragma once
#include "rasterizer.h"

namespace cave {

class ExampleBase {
public:
    ExampleBase();

    int run();

protected:
    void initialize();
    void finalize();

    virtual void update() = 0;
    // TODO: refactor getTexture()
    virtual const rs::Texture& getTexture() const { return m_renderTarget.m_colorBuffer; }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    virtual void postInit() = 0;

    rs::RenderTarget m_renderTarget;
    int m_width;
    int m_height;
};

extern ExampleBase* g_pExample;

}  // namespace cave
