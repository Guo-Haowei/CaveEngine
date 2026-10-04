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

    void submit(std::unique_ptr<render::RenderSubmission>&& p_submission) final;

    // resource
    void updateBufferData(const GpuBufferDesc& p_desc, const GpuStructuredBuffer* p_buffer) override;

    void updateBuffer(const GpuBufferDesc& p_desc, GpuBuffer* p_buffer) override;

    auto createMesh(const MeshAsset& p_mesh) -> Result<Ref<GpuMesh>> override;

    void setPipelineState(PipelineStateName p_name) override;

    Ref<GpuTexture> createTexture(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) override;
    Ref<GpuTexture> createTexture(ImageAsset* p_image) override;

    void requestTexture(ImageAsset* p_image) override;
    void requestMesh(MeshAsset* p_mesh) override;

    void beginEvent(std::string_view p_event) override { unused(p_event); }
    void endEvent() override {}

    FrameContext& getCurrentFrame() override { return *(m_frameContexts[m_frameIndex].get()); }

    void drawSkybox() override;

    void eventReceived(Ref<IEvent> p_event) final;

protected:
    virtual auto InitializeInternal() -> Result<void> = 0;
    void beginFrame() override;
    void endFrame() override;
    void moveToNextFrame() override;
    Ref<FrameContext> createFrameContext() override;

    bool m_enableValidationLayer;

    ConcurrentQueue<ImageAsset*> m_loadedImages;
    ConcurrentQueue<MeshAsset*> m_loadedMeshes;

    Ref<PipelineStateManager> m_pipelineStateManager;
    std::vector<Ref<FrameContext>> m_frameContexts;
    int m_frameIndex{ 0 };
    const int m_frameCount;

    Ref<GpuMesh> m_screenQuadBuffers;
    Ref<GpuMesh> m_skybox_buffers;

protected:
    void updateEmitters(const Scene& p_scene) override;

    void beginPass(const CompiledPass& p_pass) override;
    void endPass(const CompiledPass& p_pass) override;

private:
    void Execute(const FrameData& p_data, const CompiledPass& p_pass);
};

}  // namespace cave::render
