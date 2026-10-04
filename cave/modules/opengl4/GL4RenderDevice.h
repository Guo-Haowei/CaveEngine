#pragma once

#include "../opengl_common/GLRenderDevice.h"

struct GLFWwindow;

namespace cave::render {

class GL4RenderDevice : public GLRenderDevice {
public:
    GL4RenderDevice()
        : GLRenderDevice() {}

    void Dispatch(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) final;
    void BindUnorderedAccessView(uint32_t slot, GpuTexture* texture) final;
    void UnbindUnorderedAccessView(uint32_t slot) final;

    void BindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) final;
    void UnbindStructuredBuffer(int slot) final;

    auto CreateStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> final;
    void UpdateBufferData(const GpuBufferDesc& desc, const GpuStructuredBuffer* buffer) final;

    void beginEvent(std::string_view event) final;
    void endEvent() final;

protected:
    auto InitializeInternal() -> Result<void> final;
};

}  // namespace cave::render
