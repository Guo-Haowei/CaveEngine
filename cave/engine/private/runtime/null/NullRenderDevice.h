#pragma once
#include "engine/private/renderer/graphics_defines.h"
#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave::render {

WARNING_PUSH()
WARNING_DISABLE(4100, "-Wunused-parameter")

class NullRenderDevice : public IRenderDevice {
public:
    NullRenderDevice(std::string_view p_name = "EmptyRenderDevice")
        : IRenderDevice(p_name, rhi::Backend::Null) {}

    auto InitializeImpl() -> Result<void> override { return Result<void>(); }
    void FinalizeImpl() override {}

    void submit(std::unique_ptr<render::RenderSubmission>&&) override {}

    // resource
    auto CreateConstantBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuConstantBuffer>> override { return nullptr; }
    auto CreateStructuredBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuStructuredBuffer>> override { return nullptr; }
    void UpdateBufferData(const GpuBufferDesc& p_desc, const GpuStructuredBuffer* p_buffer) override {}

    void SetRenderTargets(const RenderTargetDesc&) override {}
    void UnsetRenderTargets() override {}

    void Clear(const RenderTargetDesc&) override {}

    void SetViewport(const Viewport& p_viewport) override {}

    auto CreateBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuBuffer>> override { return nullptr; }
    void UpdateBuffer(const GpuBufferDesc& p_desc, GpuBuffer* p_buffer) override {}

    auto CreateMesh(const MeshAsset& p_mesh) -> Result<std::shared_ptr<GpuMesh>> override { return nullptr; }
    auto CreateMeshImpl(const GpuMeshDesc& p_desc,
                        std::span<const GpuBufferDesc> p_vb_descs,
                        const GpuBufferDesc* p_ib_desc) -> Result<std::shared_ptr<GpuMesh>> final {
        return nullptr;
    }

    void SetMesh(const GpuMesh* p_mesh) override {}

    void DrawElements(uint32_t p_count, uint32_t p_offset = 0) override {}
    void DrawElementsInstanced(uint32_t p_instance_count, uint32_t p_count, uint32_t p_offset = 0) override {}
    void DrawArrays(uint32_t p_count, uint32_t p_offset = 0) override {}
    void DrawArraysInstanced(uint32_t p_instance_count, uint32_t p_count, uint32_t p_offset = 0) override {}

    void Dispatch(uint32_t p_num_groups_x, uint32_t p_num_groups_y, uint32_t p_num_groups_z) override {}
    void BindUnorderedAccessView(uint32_t p_slot, GpuTexture* p_texture) override {}
    void UnbindUnorderedAccessView(uint32_t p_slot) override {}

    void SetPipelineState(PipelineStateName p_name) override {}

    void SetStencilRef(uint32_t p_ref) override {}
    void SetBlendState(const BlendDesc& p_desc, const float* p_factor, uint32_t p_mask) override {}

    void BindStructuredBuffer(int p_slot, const GpuStructuredBuffer* p_buffer) override {}
    void UnbindStructuredBuffer(int p_slot) override {}
    void BindStructuredBufferSRV(int p_slot, const GpuStructuredBuffer* p_buffer) override {}
    void UnbindStructuredBufferSRV(int p_slot) override {}

    void UpdateConstantBuffer(const GpuConstantBuffer* p_buffer, const void* p_data, size_t p_size) override {}

    void BindConstantBufferRange(const GpuConstantBuffer* p_buffer, uint32_t p_size, uint32_t p_offset) override {}

    std::shared_ptr<GpuTexture> CreateTexture(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) override { return nullptr; }
    std::shared_ptr<GpuTexture> CreateTexture(ImageAsset* p_image) override { return nullptr; }
    void BindTexture(Dimension p_dimension, uint64_t p_handle, int p_slot) override {}
    void UnbindTexture(Dimension p_dimension, int p_slot) override {}

    void beginEvent(std::string_view p_event) override {}
    void endEvent() override {}

    void GenerateMipmap(const GpuTexture* p_texture) override {}

    void RequestTexture(ImageAsset* p_image) override {}
    void RequestMesh(MeshAsset* p_mesh) override {}

    FrameContext& GetCurrentFrame() override {
        FrameContext* context = nullptr;
        return *context;
    }

    void DrawSkybox() override {}

    void EventReceived(std::shared_ptr<IEvent> p_event) override {}

protected:
    std::shared_ptr<GpuTexture> CreateTextureImpl(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) override { return nullptr; }

    void Render() override {}
    void Present() override {}

    void BeginFrame() override {}
    void EndFrame() override {}
    void MoveToNextFrame() override {}
    std::shared_ptr<FrameContext> CreateFrameContext() override { return nullptr; }

    void beginPass(const CompiledPass& p_pass) override {}
    void endPass(const CompiledPass& p_pass) override {}

    void OnWindowResize(int p_width, int p_height) override {}
    void SetPipelineStateImpl(PipelineStateName p_name) override {}
    void UpdateEmitters(const Scene& p_scene) override {}
};

WARNING_POP()

}  // namespace cave::render
