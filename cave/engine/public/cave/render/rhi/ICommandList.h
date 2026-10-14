// =============================================================================
// File: cave/render/rhi/ICommandList.h
// =============================================================================
#pragma once
#include "cave/render/rhi/Defines.h"

namespace cave::render {

// @TODO: move these to rhi/Defines.h
enum PipelineStateName : uint8_t;
struct GpuStructuredBuffer;

class ICommandList {
public:
    virtual void draw(uint32_t count,
                      uint32_t offset = 0) = 0;

    virtual void drawInstanced(uint32_t instance_count,
                               uint32_t count,
                               uint32_t offset = 0) = 0;

    virtual void drawIndexed(uint32_t count,
                             uint32_t offset = 0) = 0;

    virtual void drawIndexedInstanced(uint32_t instance_count,
                                      uint32_t count,
                                      uint32_t offset = 0) = 0;

    virtual void setPipelineState(PipelineStateName name) = 0;

    virtual void setStencilRef(uint32_t ref) = 0;

    // @TODO: remove these
    virtual void bindTexture(Dimension dimension, uint64_t handle, int slot) = 0;
    virtual void unbindTexture(Dimension dimension, int slot) = 0;

    virtual void bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) = 0;
    virtual void unbindStructuredBuffer(int slot) = 0;
    virtual void bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) = 0;
    virtual void unbindStructuredBufferSRV(int slot) = 0;
};

}  // namespace cave::render