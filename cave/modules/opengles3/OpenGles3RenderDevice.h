#pragma once

#include "../opengl_common/OpenGlRenderDevice.h"

struct GLFWwindow;

namespace cave {

class OpenGLES3GraphicsManager : public OpenGlRenderDevice {
public:
    OpenGLES3GraphicsManager()
        : OpenGlRenderDevice() {}

protected:
    auto InitializeInternal() -> Result<void> final;
};

}  // namespace cave
