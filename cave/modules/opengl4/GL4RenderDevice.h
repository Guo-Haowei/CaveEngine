#pragma once

#include "../opengl_common/GLRenderDevice.h"

struct GLFWwindow;

namespace cave::render {

class GL4RenderDevice : public GLRenderDevice {
public:
    GL4RenderDevice()
        : GLRenderDevice() {}

    void dispatch(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) final;
    void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) final;
    void unbindUnorderedAccessView(uint32_t slot) final;

    void bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) final;
    void unbindStructuredBuffer(int slot) final;

    auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> final;
    void updateBufferData(const GpuBufferDesc& desc, const GpuStructuredBuffer* buffer) final;

    void beginEvent(std::string_view event) final;
    void endEvent() final;

protected:
    auto InitializeInternal() -> Result<void> final;
};

}  // namespace cave::render
