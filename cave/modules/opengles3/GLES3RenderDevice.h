#pragma once

#include "../opengl_common/GLRenderDevice.h"

struct GLFWwindow;

namespace cave {

class OpenGLES3GraphicsManager : public CommonOpenGLGraphicsManager {
public:
    OpenGLES3GraphicsManager()
        : CommonOpenGLGraphicsManager() {}

protected:
    auto InitializeInternal() -> Result<void> final;
};

}  // namespace cave
