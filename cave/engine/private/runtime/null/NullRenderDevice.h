#pragma once
#include "engine/private/renderer/graphics_defines.h"
#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave::render {

class NullRenderDevice : public IRenderDevice {
public:
    NullRenderDevice(std::string_view name = "EmptyRenderDevice")
        : IRenderDevice(name, rhi::Backend::Null) {}

    auto InitializeImpl() -> Result<void> override { return Result<void>(); }
    void FinalizeImpl() override {}

    void submit(Owner<render::RenderSubmission>&&) override {}

    // resource
    auto CreateConstantBuffer(const GpuBufferDesc&) -> Result<Ref<GpuConstantBuffer>> override { return nullptr; }
    auto CreateStructuredBuffer(const GpuBufferDesc&) -> Result<Ref<GpuStructuredBuffer>> override { return nullptr; }
    void UpdateBufferData(const GpuBufferDesc&, const GpuStructuredBuffer*) override {}

    void SetRenderTargets(const RenderTargetDesc&) override {}
    void UnsetRenderTargets() override {}

    void Clear(const RenderTargetDesc&) override {}

    void SetViewport(const Viewport&) override {}

    auto CreateBuffer(const GpuBufferDesc&) -> Result<Ref<GpuBuffer>> override { return nullptr; }
    void UpdateBuffer(const GpuBufferDesc&, GpuBuffer*) override {}

    auto CreateMesh(const MeshAsset&) -> Result<Ref<GpuMesh>> override { return nullptr; }
    auto CreateMeshImpl(const GpuMeshDesc&,
                        std::span<const GpuBufferDesc>,
                        const GpuBufferDesc*) -> Result<Ref<GpuMesh>> final {
        return nullptr;
    }

    void SetMesh(const GpuMesh*) override {}

    void DrawElements(uint32_t, uint32_t) override {}
    void DrawElementsInstanced(uint32_t, uint32_t, uint32_t) override {}
    void DrawArrays(uint32_t, uint32_t) override {}
    void DrawArraysInstanced(uint32_t, uint32_t, uint32_t) override {}

    void Dispatch(uint32_t, uint32_t, uint32_t) override {}
    void BindUnorderedAccessView(uint32_t, GpuTexture*) override {}
    void UnbindUnorderedAccessView(uint32_t) override {}

    void SetPipelineState(PipelineStateName) override {}

    void SetStencilRef(uint32_t) override {}
    void SetBlendState(const BlendDesc&, const float*, uint32_t) override {}

    void BindStructuredBuffer(int, const GpuStructuredBuffer*) override {}
    void UnbindStructuredBuffer(int) override {}
    void BindStructuredBufferSRV(int, const GpuStructuredBuffer*) override {}
    void UnbindStructuredBufferSRV(int) override {}

    void UpdateConstantBuffer(const GpuConstantBuffer*, const void*, size_t) override {}

    void BindConstantBufferRange(const GpuConstantBuffer*, uint32_t, uint32_t) override {}

    Ref<GpuTexture> CreateTexture(const GpuTextureDesc&, const SamplerDesc&) override { return nullptr; }
    Ref<GpuTexture> CreateTexture(ImageAsset*) override { return nullptr; }
    void BindTexture(Dimension, uint64_t, int) override {}
    void UnbindTexture(Dimension, int) override {}

    void beginEvent(std::string_view) override {}
    void endEvent() override {}

    void GenerateMipmap(const GpuTexture*) override {}

    void RequestTexture(ImageAsset*) override {}
    void RequestMesh(MeshAsset*) override {}

    FrameContext& GetCurrentFrame() override {
        FrameContext* context = nullptr;
        return *context;
    }

    void DrawSkybox() override {}

    void EventReceived(Ref<IEvent>) override {}

protected:
    Ref<GpuTexture> CreateTextureImpl(const GpuTextureDesc&, const SamplerDesc&) override { return nullptr; }

    void Render() override {}
    void Present() override {}

    void BeginFrame() override {}
    void EndFrame() override {}
    void MoveToNextFrame() override {}

    Ref<FrameContext> CreateFrameContext() override { return nullptr; }

    void beginPass(const CompiledPass&) override {}
    void endPass(const CompiledPass&) override {}

    void OnWindowResize(int, int) override {}
    void SetPipelineStateImpl(PipelineStateName) override {}
    void UpdateEmitters(const Scene&) override {}
};

}  // namespace cave::render
