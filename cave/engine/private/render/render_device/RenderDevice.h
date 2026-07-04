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
    std::shared_ptr<GpuConstantBuffer> batchCb;
    std::shared_ptr<GpuConstantBuffer> materialCb;
    std::shared_ptr<GpuConstantBuffer> boneCb;
    std::shared_ptr<GpuConstantBuffer> passCb;
    std::shared_ptr<GpuConstantBuffer> emitterCb;
    std::shared_ptr<GpuConstantBuffer> pointShadowCb;
    std::shared_ptr<GpuConstantBuffer> perFrameCb;
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
    void UpdateBufferData(const GpuBufferDesc& p_desc, const GpuStructuredBuffer* p_buffer) override;

    void UpdateBuffer(const GpuBufferDesc& p_desc, GpuBuffer* p_buffer) override;

    auto CreateMesh(const MeshAsset& p_mesh) -> Result<std::shared_ptr<GpuMesh>> override;

    void SetPipelineState(PipelineStateName p_name) override;

    std::shared_ptr<GpuTexture> CreateTexture(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) override;
    std::shared_ptr<GpuTexture> CreateTexture(ImageAsset* p_image) override;

    void RequestTexture(ImageAsset* p_image) override;
    void RequestMesh(MeshAsset* p_mesh) override;

    void BeginEvent(std::string_view p_event) override { unused(p_event); }
    void EndEvent() override {}

    FrameContext& GetCurrentFrame() override { return *(m_frameContexts[m_frameIndex].get()); }

    void DrawSkybox() override;

    void EventReceived(std::shared_ptr<IEvent> p_event) final;

protected:
    virtual auto InitializeInternal() -> Result<void> = 0;
    void BeginFrame() override;
    void EndFrame() override;
    void MoveToNextFrame() override;
    std::shared_ptr<FrameContext> CreateFrameContext() override;

    bool m_enableValidationLayer;

    ConcurrentQueue<ImageAsset*> m_loadedImages;
    ConcurrentQueue<MeshAsset*> m_loadedMeshes;

    std::shared_ptr<PipelineStateManager> m_pipelineStateManager;
    std::vector<std::shared_ptr<FrameContext>> m_frameContexts;
    int m_frameIndex{ 0 };
    const int m_frameCount;

    std::shared_ptr<GpuMesh> m_screenQuadBuffers;
    std::shared_ptr<GpuMesh> m_skybox_buffers;

    // auto InitializeInternal() -> Result<void> final;
    // std::shared_ptr<GpuTexture> CreateTextureImpl(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) final;

protected:
    void UpdateEmitters(const Scene& p_scene) override;

    void BeginPass(const CompiledPass& p_pass) override;
    void EndPass(const CompiledPass& p_pass) override;

private:
    void Execute(const FrameData& p_data, const CompiledPass& p_pass);
};

}  // namespace cave::render
