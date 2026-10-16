#pragma once
#include "engine/private/render/renderer/GraphicsDefines.h"
#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave::render {

class NullRenderDevice : public IRenderDevice {
public:
    NullRenderDevice(std::string_view name = "EmptyRenderDevice")
        : IRenderDevice(name, Backend::Null) {}

    auto InitializeImpl() -> Result<void> override { return Result<void>(); }
    void FinalizeImpl() override {}

    void submit(Owner<render::RenderSubmission>&&) override {}

    // resource
    auto createConstantBuffer(const GpuResourceDesc&) -> Result<Ref<GpuBuffer>> override { return nullptr; }
    auto createStructuredBuffer(const GpuResourceDesc&) -> Result<Ref<GpuBuffer>> override { return nullptr; }
    void updateBufferData(const GpuResourceDesc&, const GpuBuffer*) override {}

    auto createBuffer(const GpuResourceDesc&) -> Result<Ref<GpuBuffer>> override { return nullptr; }
    void updateBuffer(const GpuResourceDesc&, GpuBuffer*) override {}

    auto createMesh(const MeshAsset&) -> Result<Ref<GpuMesh>> override { return nullptr; }
    auto createMeshImpl(const GpuMeshDesc&,
                        std::span<const GpuResourceDesc>,
                        const GpuResourceDesc*) -> Result<Ref<GpuMesh>> final {
        return nullptr;
    }

    void updateConstantBuffer(const GpuBuffer*, const void*, size_t) override {}

    Ref<GpuTexture> createTexture(const GpuResourceDesc&, const SamplerDesc&) override { return nullptr; }
    Ref<GpuTexture> createTexture(ImageAsset*) override { return nullptr; }

    void generateMipmap(const GpuTexture*) override {}

    void requestTexture(ImageAsset*) override {}
    void requestMesh(MeshAsset*) override {}

    FrameContext& getCurrentFrame() override {
        FrameContext* context = nullptr;
        return *context;
    }

    void drawSkybox() override {}

    void eventReceived(Ref<IEvent>) override {}

    ICommandList* getCommandList() const override { return nullptr; }

protected:
    Ref<GpuTexture> createTextureImpl(const GpuResourceDesc&, const SamplerDesc&) override { return nullptr; }

    void render() override {}
    void present() override {}

    void beginFrame() override {}
    void endFrame() override {}
    void moveToNextFrame() override {}

    Ref<FrameContext> createFrameContext() override { return nullptr; }

    void onWindowResize(int, int) override {}
    void updateEmitters(const Scene&) override {}
};

}  // namespace cave::render
