// =============================================================================
// File: cave/render/rhi/ICommandList.h
// =============================================================================
#pragma once
#include "cave/render/rhi/Defines.h"

namespace cave::render {

// @TODO: move these to rhi/Defines.h
enum PipelineStateName : uint8_t;

struct GpuConstantBuffer;
struct GpuMesh;
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

    virtual void beginRendering(const RenderingInfo& info) = 0;
    virtual void endRendering() = 0;

    virtual void pushDebugGroup(std::string_view name) = 0;
    virtual void popDebugGroup() = 0;

    virtual void setViewport(const Viewport& viewport) = 0;

    virtual void setPipelineState(PipelineStateName name) = 0;

    virtual void setStencilRef(uint32_t ref) = 0;

    virtual void bindConstantBufferRange(const GpuConstantBuffer* buffer,
                                         uint32_t size,
                                         uint32_t offset) = 0;

    template<typename T>
    void bindConstantBufferSlot(const GpuConstantBuffer* buffer, int slot) {
        bindConstantBufferRange(buffer, sizeof(T), slot * sizeof(T));
    }

    virtual void dispatch(uint32_t num_groups_x,
                          uint32_t num_groups_y,
                          uint32_t num_groups_z) = 0;

    // @TODO: remove these
    virtual void setMesh(const GpuMesh* mesh) = 0;

    virtual void bindTexture(Dimension dimension, uint64_t handle, int slot) = 0;
    virtual void unbindTexture(Dimension dimension, int slot) = 0;

    virtual void bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) = 0;
    virtual void unbindStructuredBuffer(int slot) = 0;
    virtual void bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) = 0;
    virtual void unbindStructuredBufferSRV(int slot) = 0;

    virtual void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) = 0;
    virtual void unbindUnorderedAccessView(uint32_t slot) = 0;
};

}  // namespace cave::render