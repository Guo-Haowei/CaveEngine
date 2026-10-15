#pragma once
#include "cave/core/base/Singleton.h"

#include "engine/private/render/render_graph/CompiledGraph.h"

#include "engine/private/core/base/concurrent_queue.h"
#include "engine/private/core/math/Geomath.h"
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/render/rhi/PipelineState.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/framework/PipelineStateManager.h"

namespace cave {
#include "cbuffer.slang.h"
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
    Ref<GpuConstantBuffer> batch_cb;
    Ref<GpuConstantBuffer> material_cb;
    Ref<GpuConstantBuffer> bone_cb;
    Ref<GpuConstantBuffer> view_cb;
    Ref<GpuConstantBuffer> pass_cb;
    Ref<GpuConstantBuffer> emitter_cb;
    Ref<GpuConstantBuffer> per_frame_cb;
    Ref<GpuConstantBuffer> point_shadow_cb;
};

class RenderDevice : public IRenderDevice,
                     public Singleton<RenderDevice> {
public:
    RenderDevice(std::string_view name, Backend backend, int frame_count)
        : IRenderDevice(name, backend), m_frame_count(frame_count) {}

    auto InitializeImpl() -> Result<void> final;

    void submit(Owner<render::RenderSubmission>&& submission) final;

    // resource
    void updateBufferData(const GpuBufferDesc& desc, const GpuStructuredBuffer* buffer) override;

    void updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) override;

    auto createMesh(const MeshAsset& mesh) -> Result<Ref<GpuMesh>> override;

    Ref<GpuTexture> createTexture(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) override;
    Ref<GpuTexture> createTexture(ImageAsset* image) override;

    void requestTexture(ImageAsset* image) override;
    void requestMesh(MeshAsset* mesh) override;

    FrameContext& getCurrentFrame() override {
        return *(m_frame_contexts[m_frame_index].get());
    }

    void drawSkybox() override;

    void eventReceived(Ref<IEvent> event) final;

private:
    void execute(const FrameData& data, const CompiledPass& pass);
    void beginPass(const CompiledPass& pass);
    void endPass(const CompiledPass& pass);

protected:
    virtual auto InitializeInternal() -> Result<void> = 0;
    void beginFrame() override;
    void endFrame() override;
    void moveToNextFrame() override;
    Ref<FrameContext> createFrameContext() override;
    void updateEmitters(const Scene& scene) override;

    bool m_enable_validation_layer;

    ConcurrentQueue<ImageAsset*> m_loaded_images;
    ConcurrentQueue<MeshAsset*> m_loaded_meshes;

    Owner<PipelineStateManager> m_pipeline_state_manager;
    std::vector<Ref<FrameContext>> m_frame_contexts;
    int m_frame_index{ 0 };
    const int m_frame_count;

    Ref<GpuMesh> m_screen_quad_buffers;
    Ref<GpuMesh> m_skybox_buffers;
};

}  // namespace cave::render
