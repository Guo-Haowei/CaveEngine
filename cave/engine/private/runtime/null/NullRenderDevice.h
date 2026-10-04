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
    auto createConstantBuffer(const GpuBufferDesc&) -> Result<Ref<GpuConstantBuffer>> override { return nullptr; }
    auto createStructuredBuffer(const GpuBufferDesc&) -> Result<Ref<GpuStructuredBuffer>> override { return nullptr; }
    void updateBufferData(const GpuBufferDesc&, const GpuStructuredBuffer*) override {}

    void setRenderTargets(const RenderTargetDesc&) override {}
    void unsetRenderTargets() override {}

    void clear(const RenderTargetDesc&) override {}

    void setViewport(const Viewport&) override {}

    auto createBuffer(const GpuBufferDesc&) -> Result<Ref<GpuBuffer>> override { return nullptr; }
    void updateBuffer(const GpuBufferDesc&, GpuBuffer*) override {}

    auto createMesh(const MeshAsset&) -> Result<Ref<GpuMesh>> override { return nullptr; }
    auto createMeshImpl(const GpuMeshDesc&,
                        std::span<const GpuBufferDesc>,
                        const GpuBufferDesc*) -> Result<Ref<GpuMesh>> final {
        return nullptr;
    }

    void setMesh(const GpuMesh*) override {}

    void drawElements(uint32_t, uint32_t) override {}
    void drawElementsInstanced(uint32_t, uint32_t, uint32_t) override {}
    void drawArrays(uint32_t, uint32_t) override {}
    void drawArraysInstanced(uint32_t, uint32_t, uint32_t) override {}

    void dispatch(uint32_t, uint32_t, uint32_t) override {}
    void bindUnorderedAccessView(uint32_t, GpuTexture*) override {}
    void unbindUnorderedAccessView(uint32_t) override {}

    void setPipelineState(PipelineStateName) override {}

    void setStencilRef(uint32_t) override {}
    void setBlendState(const BlendDesc&, const float*, uint32_t) override {}

    void bindStructuredBuffer(int, const GpuStructuredBuffer*) override {}
    void unbindStructuredBuffer(int) override {}
    void bindStructuredBufferSRV(int, const GpuStructuredBuffer*) override {}
    void unbindStructuredBufferSRV(int) override {}

    void updateConstantBuffer(const GpuConstantBuffer*, const void*, size_t) override {}

    void bindConstantBufferRange(const GpuConstantBuffer*, uint32_t, uint32_t) override {}

    Ref<GpuTexture> createTexture(const GpuTextureDesc&, const SamplerDesc&) override { return nullptr; }
    Ref<GpuTexture> createTexture(ImageAsset*) override { return nullptr; }
    void bindTexture(Dimension, uint64_t, int) override {}
    void unbindTexture(Dimension, int) override {}

    void beginEvent(std::string_view) override {}
    void endEvent() override {}

    void generateMipmap(const GpuTexture*) override {}

    void requestTexture(ImageAsset*) override {}
    void requestMesh(MeshAsset*) override {}

    FrameContext& getCurrentFrame() override {
        FrameContext* context = nullptr;
        return *context;
    }

    void drawSkybox() override {}

    void eventReceived(Ref<IEvent>) override {}

protected:
    Ref<GpuTexture> createTextureImpl(const GpuTextureDesc&, const SamplerDesc&) override { return nullptr; }

    void render() override {}
    void present() override {}

    void beginFrame() override {}
    void endFrame() override {}
    void moveToNextFrame() override {}

    Ref<FrameContext> createFrameContext() override { return nullptr; }

    void beginPass(const CompiledPass&) override {}
    void endPass(const CompiledPass&) override {}

    void onWindowResize(int, int) override {}
    void setPipelineStateImpl(PipelineStateName) override {}
    void updateEmitters(const Scene&) override {}
};

}  // namespace cave::render
