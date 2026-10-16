#pragma once

#include "../opengl_common/OpenGlRenderDevice.h"

namespace cave::render {

class OpenGl4RenderDevice final : public OpenGlRenderDevice {
public:
    OpenGl4RenderDevice()
        : OpenGlRenderDevice() {}

    auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> override;
    void updateBufferData(const GpuBufferDesc& desc, const GpuBuffer* buffer) override;

protected:
    auto InitializeInternal() -> Result<void> override;
};

}  // namespace cave::render
