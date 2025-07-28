#pragma once
#include "sampler.h"

namespace cave::rs {

class RenderTarget {
public:
    struct CreateInfo {
        int width;
        int height;
        bool useColor;
        bool useDepth;
    };

    void create(const CreateInfo& info);

    void resize(int width, int height);

    const auto& getColorBuffer() const { return m_colorBuffer; }

    const auto& getDepthBuffer() const { return m_depthBuffer; }

public:
    TextureBase<Vector4f> m_colorBuffer;
    DepthBuffer m_depthBuffer;

    bool m_useColor = true;
    bool m_useDepth = true;
};

}  // namespace cave::rs
