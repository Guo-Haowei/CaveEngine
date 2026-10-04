#pragma once
#include "cave/core/base/Singleton.h"

#include "engine/private/render/rhi/RenderTarget.h"
#include "engine/private/render/render_graph/CompiledGraph.h"

#include "engine/private/core/base/concurrent_queue.h"
#include "engine/private/core/math/geomath.h"
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/render/rhi/PipelineState.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/framework/PipelineStateManager.h"

namespace cave {
#include "cbuffer.hlsl.h"
}  // namespace cave

// @TODO: refactor
struct MaterialConstantBuffer;

namespace cave {

struct SamplerDesc;
class Scene;
struct GpuConstantBuffer;
}  // namespace cave

namespace cave::render {

struct CompiledPass;

struct FrameContext {
    Ref<GpuConstantBuffer> batchCb;
    Ref<GpuConstantBuffer> materialCb;
    Ref<GpuConstantBuffer> boneCb;
    Ref<GpuConstantBuffer> passCb;
    Ref<GpuConstantBuffer> emitterCb;
    Ref<GpuConstantBuffer> pointShadowCb;
    Ref<GpuConstantBuffer> perFrameCb;
};

class RenderDevice : public IRenderDevice,
                     public Singleton<RenderDevice> {
public:
    // @TODO: rename to RenderTarget

    RenderDevice(std::string_view name, rhi::Backend backend, int frame_count)
        : IRenderDevice(name, backend), m_frameCount(frame_count) {}

    auto InitializeImpl() -> Result<void> final;

    void submit(Owner<render::RenderSubmission>&& submission) final;

    // resource
    void updateBufferData(const GpuBufferDesc& desc, const GpuStructuredBuffer* buffer) override;

    void updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) override;

    auto createMesh(const MeshAsset& mesh) -> Result<Ref<GpuMesh>> override;

    void setPipelineState(PipelineStateName name) override;

    Ref<GpuTexture> createTexture(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) override;
    Ref<GpuTexture> createTexture(ImageAsset* image) override;

    void requestTexture(ImageAsset* image) override;
    void requestMesh(MeshAsset* mesh) override;

    void beginEvent(std::string_view event) override { unused(event); }
    void endEvent() override {}

    FrameContext& getCurrentFrame() override { return *(m_frameContexts[m_frameIndex].get()); }

    void drawSkybox() override;

    void eventReceived(Ref<IEvent> event) final;

protected:
    virtual auto InitializeInternal() -> Result<void> = 0;
    void beginFrame() override;
    void endFrame() override;
    void moveToNextFrame() override;
    Ref<FrameContext> createFrameContext() override;

    bool m_enableValidationLayer;

    ConcurrentQueue<ImageAsset*> m_loadedImages;
    ConcurrentQueue<MeshAsset*> m_loadedMeshes;

    Owner<PipelineStateManager> m_pipeline_state_manager;
    std::vector<Ref<FrameContext>> m_frameContexts;
    int m_frameIndex{ 0 };
    const int m_frameCount;

    Ref<GpuMesh> m_screenQuadBuffers;
    Ref<GpuMesh> m_skybox_buffers;

protected:
    void updateEmitters(const Scene& scene) override;

    void beginPass(const CompiledPass& pass) override;
    void endPass(const CompiledPass& pass) override;

private:
    void Execute(const FrameData& data, const CompiledPass& pass);
};

}  // namespace cave::render
