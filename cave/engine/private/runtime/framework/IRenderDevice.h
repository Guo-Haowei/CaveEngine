#pragma once
#include "cave/runtime/framework/IService.h"
#include "cave/rhi/Backend.h"
#include "engine/private/runtime/framework/EventQueue.h"

// @TODO: refactor
struct MaterialConstantBuffer;

namespace cave {

enum ClearFlags : uint32_t;
enum class Dimension : uint8_t;
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

struct RenderCapabilities {
    bool supportComputeShaders = true;
    bool supportGeometryShaders = true;
    bool supportStructuredBuffers = true;
};

// @TODO: split this class to RenderDevice and RHI
class IRenderDevice : public IService,
                      public EventListener,
                      public ServiceCreateRegistry<IRenderDevice> {
public:
    static constexpr int NUM_FRAMES_IN_FLIGHT = 2;
    static constexpr int NUM_BACK_BUFFERS = 2;

    IRenderDevice(std::string_view name, rhi::Backend backend)
        : IService(name)
        , m_backend(backend) {}

    virtual auto InitializeImpl() -> Result<void> = 0;

    virtual void submit(Owner<RenderSubmission>&& submission) = 0;

    // resource
    virtual auto createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuConstantBuffer>> = 0;
    virtual auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> = 0;
    virtual void updateBufferData(const GpuBufferDesc& desc, const GpuStructuredBuffer* buffer) = 0;

    virtual void setRenderTargets(const RenderTargetDesc& desc) = 0;
    virtual void unsetRenderTargets() = 0;

    virtual void clear(const RenderTargetDesc& framebuffer) = 0;

    virtual void setViewport(const Viewport& viewport) = 0;

    virtual auto createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> = 0;
    virtual void updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) = 0;

    virtual auto createMesh(const MeshAsset& mesh) -> Result<Ref<GpuMesh>> = 0;

    virtual auto createMeshImpl(const GpuMeshDesc& desc,
                                std::span<const GpuBufferDesc> vb_descs,
                                const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> = 0;

    virtual void setMesh(const GpuMesh* mesh) = 0;

    virtual void drawElements(uint32_t count, uint32_t offset = 0) = 0;
    virtual void drawElementsInstanced(uint32_t instance_count, uint32_t count, uint32_t offset = 0) = 0;
    virtual void drawArrays(uint32_t count, uint32_t offset = 0) = 0;
    virtual void drawArraysInstanced(uint32_t instance_count, uint32_t count, uint32_t offset = 0) = 0;

    virtual void dispatch(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) = 0;
    virtual void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) = 0;
    virtual void unbindUnorderedAccessView(uint32_t slot) = 0;

    virtual void setPipelineState(PipelineStateName name) = 0;

    virtual void setStencilRef(uint32_t ref) = 0;
    virtual void setBlendState(const BlendDesc& desc, const float* factor, uint32_t mask) = 0;

    virtual void bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) = 0;
    virtual void unbindStructuredBuffer(int slot) = 0;
    virtual void bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) = 0;
    virtual void unbindStructuredBufferSRV(int slot) = 0;

    virtual void updateConstantBuffer(const GpuConstantBuffer* buffer, const void* data, size_t size) = 0;
    template<typename T>
    void updateConstantBuffer(const GpuConstantBuffer* buffer, const std::vector<T>& vector) {
        updateConstantBuffer(buffer, vector.data(), sizeof(T) * (uint32_t)vector.size());
    }
    template<typename T, int N>
    void updateConstantBuffer(const GpuConstantBuffer* buffer, const std::array<T, N>& array) {
        updateConstantBuffer(buffer, array.data(), sizeof(T) * N);
    }

    virtual void bindConstantBufferRange(const GpuConstantBuffer* buffer, uint32_t size, uint32_t offset) = 0;
    template<typename T>
    void bindConstantBufferSlot(const GpuConstantBuffer* buffer, int slot) {
        bindConstantBufferRange(buffer, sizeof(T), slot * sizeof(T));
    }

    virtual Ref<GpuTexture> createTexture(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) = 0;
    virtual Ref<GpuTexture> createTexture(ImageAsset* image) = 0;
    virtual void bindTexture(Dimension dimension, uint64_t handle, int slot) = 0;
    virtual void unbindTexture(Dimension dimension, int slot) = 0;

    virtual void generateMipmap(const GpuTexture* texture) = 0;

    virtual void beginEvent(std::string_view event) = 0;
    virtual void endEvent() = 0;

    virtual void requestTexture(ImageAsset* image) = 0;
    virtual void requestMesh(MeshAsset* mesh) = 0;

    // @TODO: thread safety ?
    virtual void eventReceived(Ref<IEvent> event) = 0;

    virtual FrameContext& getCurrentFrame() = 0;

    virtual void drawSkybox() = 0;

    rhi::Backend backend() const { return m_backend; }

    const RenderCapabilities& getCapabilities() const { return m_capabilities; }

protected:
    virtual Ref<GpuTexture> createTextureImpl(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) = 0;

    virtual void render() = 0;
    virtual void present() = 0;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void moveToNextFrame() = 0;
    virtual Ref<FrameContext> createFrameContext() = 0;

    virtual void beginPass(const CompiledPass& pass) = 0;
    virtual void endPass(const CompiledPass& pass) = 0;

    virtual void onWindowResize(int width, int height) = 0;
    virtual void setPipelineStateImpl(PipelineStateName name) = 0;

protected:
    virtual void updateEmitters(const Scene& scene) = 0;

    RenderCapabilities m_capabilities{};

private:
    rhi::Backend m_backend;
};

}  // namespace cave::render
