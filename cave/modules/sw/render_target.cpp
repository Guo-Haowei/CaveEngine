#include "render_target.h"

namespace cave {

void SwRenderTarget::resize(int width, int height) {
    m_colorBuffer.resize(width, height);
    m_depthBuffer.resize(width, height);
}

void SwRenderTarget::create(const CreateInfo& info) {
    resize(info.width, info.height);
    m_useColor = info.useColor;
    m_useDepth = info.useDepth;
}

}  // namespace cave
