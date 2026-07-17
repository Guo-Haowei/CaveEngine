#pragma once
#include "cave/runtime/framework/IService.h"
#include "cave/rhi/Backend.h"
#include "engine/private/runtime/framework/EventQueue.h"

// @TODO: refactor
struct MaterialConstantBuffer;

namespace cave {

enum ClearFlags : uint32_t;
enum class Dimension : uint32_t;
enum PipelineStateName : uint8_t;

class Scene;

struct BlendDesc;
struct GpuBuffer;
struct GpuBufferDesc;
struct GpuConstantBuffer;
struct GpuMeshDesc;
struct GpuStructuredBuffer;
struct GpuTexture;
struct GpuTextureDesc;
struct ImageAsset;
class MeshAsset;
struct SamplerDesc;
struct Viewport;

struct GpuMesh;

}  // namespace cave

namespace cave::render {

struct RenderTargetDesc;
struct FrameContext;
struct RenderSubmission;
struct CompiledPass;

// @TODO: split this class to RenderDevice and RHI
class IRenderDevice : public IService,
                      public EventListener,
                      public ServiceCreateRegistry<IRenderDevice> {
public:
    static constexpr int NUM_FRAMES_IN_FLIGHT = 2;
    static constexpr int NUM_BACK_BUFFERS = 2;

    IRenderDevice(std::string_view name, rhi::Backend backend)
        : IService(name)
        , backend_(backend) {}

    virtual auto InitializeImpl() -> Result<void> = 0;

    virtual void submit(std::unique_ptr<RenderSubmission>&& p_submission) = 0;

    // resource
    virtual auto CreateConstantBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuConstantBuffer>> = 0;
    virtual auto CreateStructuredBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuStructuredBuffer>> = 0;
    virtual void UpdateBufferData(const GpuBufferDesc& p_desc, const GpuStructuredBuffer* p_buffer) = 0;

    virtual void SetRenderTargets(const RenderTargetDesc& p_desc) = 0;
    virtual void UnsetRenderTargets() = 0;

    virtual void Clear(const RenderTargetDesc& p_framebuffer) = 0;

    virtual void SetViewport(const Viewport& p_viewport) = 0;

    virtual auto CreateBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuBuffer>> = 0;
    virtual void UpdateBuffer(const GpuBufferDesc& p_desc, GpuBuffer* p_buffer) = 0;

    virtual auto CreateMesh(const MeshAsset& p_mesh) -> Result<std::shared_ptr<GpuMesh>> = 0;

    virtual auto CreateMeshImpl(const GpuMeshDesc& p_desc,
                                std::span<const GpuBufferDesc> p_vb_descs,
                                const GpuBufferDesc* p_ib_desc) -> Result<std::shared_ptr<GpuMesh>> = 0;

    virtual void SetMesh(const GpuMesh* p_mesh) = 0;

    virtual void DrawElements(uint32_t p_count, uint32_t p_offset = 0) = 0;
    virtual void DrawElementsInstanced(uint32_t p_instance_count, uint32_t p_count, uint32_t p_offset = 0) = 0;
    virtual void DrawArrays(uint32_t p_count, uint32_t p_offset = 0) = 0;
    virtual void DrawArraysInstanced(uint32_t p_instance_count, uint32_t p_count, uint32_t p_offset = 0) = 0;

    virtual void Dispatch(uint32_t p_num_groups_x, uint32_t p_num_groups_y, uint32_t p_num_groups_z) = 0;
    virtual void BindUnorderedAccessView(uint32_t p_slot, GpuTexture* p_texture) = 0;
    virtual void UnbindUnorderedAccessView(uint32_t p_slot) = 0;

    virtual void SetPipelineState(PipelineStateName p_name) = 0;

    virtual void SetStencilRef(uint32_t p_ref) = 0;
    virtual void SetBlendState(const BlendDesc& p_desc, const float* p_factor, uint32_t p_mask) = 0;

    virtual void BindStructuredBuffer(int p_slot, const GpuStructuredBuffer* p_buffer) = 0;
    virtual void UnbindStructuredBuffer(int p_slot) = 0;
    virtual void BindStructuredBufferSRV(int p_slot, const GpuStructuredBuffer* p_buffer) = 0;
    virtual void UnbindStructuredBufferSRV(int p_slot) = 0;

    virtual void UpdateConstantBuffer(const GpuConstantBuffer* p_buffer, const void* p_data, size_t p_size) = 0;
    template<typename T>
    void UpdateConstantBuffer(const GpuConstantBuffer* p_buffer, const std::vector<T>& p_vector) {
        UpdateConstantBuffer(p_buffer, p_vector.data(), sizeof(T) * (uint32_t)p_vector.size());
    }
    template<typename T, int N>
    void UpdateConstantBuffer(const GpuConstantBuffer* p_buffer, const std::array<T, N>& p_array) {
        UpdateConstantBuffer(p_buffer, p_array.data(), sizeof(T) * N);
    }

    virtual void BindConstantBufferRange(const GpuConstantBuffer* p_buffer, uint32_t p_size, uint32_t p_offset) = 0;
    template<typename T>
    void BindConstantBufferSlot(const GpuConstantBuffer* p_buffer, int slot) {
        BindConstantBufferRange(p_buffer, sizeof(T), slot * sizeof(T));
    }

    virtual std::shared_ptr<GpuTexture> CreateTexture(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) = 0;
    virtual std::shared_ptr<GpuTexture> CreateTexture(ImageAsset* p_image) = 0;
    virtual void BindTexture(Dimension p_dimension, uint64_t p_handle, int p_slot) = 0;
    virtual void UnbindTexture(Dimension p_dimension, int p_slot) = 0;

    virtual void GenerateMipmap(const GpuTexture* p_texture) = 0;

    virtual void beginEvent(std::string_view p_event) = 0;
    virtual void endEvent() = 0;

    virtual void RequestTexture(ImageAsset* p_image) = 0;
    virtual void RequestMesh(MeshAsset* p_mesh) = 0;

    // @TODO: thread safety ?
    virtual void EventReceived(std::shared_ptr<IEvent> p_event) = 0;

    virtual FrameContext& GetCurrentFrame() = 0;

    virtual void DrawSkybox() = 0;

    rhi::Backend backend() const { return backend_; }

protected:
    virtual std::shared_ptr<GpuTexture> CreateTextureImpl(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) = 0;

    virtual void Render() = 0;
    virtual void Present() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void MoveToNextFrame() = 0;
    virtual std::shared_ptr<FrameContext> CreateFrameContext() = 0;

    virtual void beginPass(const CompiledPass& p_pass) = 0;
    virtual void endPass(const CompiledPass& p_pass) = 0;

    virtual void OnWindowResize(int p_width, int p_height) = 0;
    virtual void SetPipelineStateImpl(PipelineStateName p_name) = 0;

protected:
    virtual void UpdateEmitters(const Scene& p_scene) = 0;

private:
    rhi::Backend backend_;
};

}  // namespace cave::render
