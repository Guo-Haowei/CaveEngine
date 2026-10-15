#pragma once

#include "../opengl_common/OpenGlRenderDevice.h"

namespace cave::render {

class OpenGl4RenderDevice final : public OpenGlRenderDevice {
public:
    OpenGl4RenderDevice()
        : OpenGlRenderDevice() {}

    auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> override;
    void updateBufferData(const GpuBufferDesc& desc, const GpuStructuredBuffer* buffer) override;

    void beginEvent(std::string_view event) override;
    void endEvent() override;

protected:
    auto InitializeInternal() -> Result<void> override;
};

}  // namespace cave::render
