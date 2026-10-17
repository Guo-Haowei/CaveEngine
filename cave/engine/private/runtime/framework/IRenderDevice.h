#pragma once
#include "cave/render/rhi/ICommandList.h"
#include "cave/runtime/framework/IService.h"

#include "engine/private/runtime/framework/EventQueue.h"

namespace cave {

class Scene;

struct ImageAsset;
class MeshAsset;

}  // namespace cave

namespace cave::render {

enum class PipelineStateName : uint8_t;

struct FrameContext;
struct RenderSubmission;

struct BlendDesc;
struct GpuResource;
struct GpuResourceDesc;
struct GpuMeshDesc;
struct GpuResource;
struct SamplerDesc;
struct Viewport;

struct GpuMesh;

// @TODO: refactor this
struct RenderCapabilities {
    bool support_cs = true;
    bool support_gs = true;
    bool support_sbo = true;
};

// @TODO: split this class to RenderDevice and RHI
class IRenderDevice : public IService,
                      public EventListener,
                      public ServiceCreateRegistry<IRenderDevice> {
public:
    static constexpr int kNumFramesInFlight = 2;
    static constexpr int kNumBackBuffers = 2;

    IRenderDevice(std::string_view name, Backend backend)
        : IService(name)
        , m_backend(backend) {}

    virtual auto InitializeImpl() -> Result<void> = 0;

    virtual void submit(Owner<RenderSubmission>&& submission) = 0;

    // resource
    virtual auto createConstantBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> = 0;
    virtual auto createStructuredBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> = 0;
    virtual void updateBufferData(const GpuResourceDesc& desc, const GpuResource* buffer) = 0;

    virtual auto createBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> = 0;
    virtual void updateBuffer(const GpuResourceDesc& desc, GpuResource* buffer) = 0;

    virtual auto createMesh(const MeshAsset& mesh) -> Result<Ref<GpuMesh>> = 0;

    virtual auto createMeshImpl(const GpuMeshDesc& desc,
                                std::span<const GpuResourceDesc> vb_descs,
                                const GpuResourceDesc* ib_desc) -> Result<Ref<GpuMesh>> = 0;

    virtual void updateConstantBuffer(const GpuResource* buffer, const void* data, size_t size) = 0;
    template<typename T>
    void updateConstantBuffer(const GpuResource* buffer, const std::vector<T>& vector) {
        updateConstantBuffer(buffer, vector.data(), sizeof(T) * (uint32_t)vector.size());
    }
    template<typename T, int N>
    void updateConstantBuffer(const GpuResource* buffer, const std::array<T, N>& array) {
        updateConstantBuffer(buffer, array.data(), sizeof(T) * N);
    }

    virtual Ref<GpuResource> createTexture(const GpuResourceDesc& texture_desc, const SamplerDesc& sampler_desc) = 0;
    virtual Ref<GpuResource> createTexture(ImageAsset* image) = 0;

    virtual void generateMipmap(const GpuResource* texture) = 0;

    virtual void requestTexture(ImageAsset* image) = 0;
    virtual void requestMesh(MeshAsset* mesh) = 0;

    // @TODO: thread safety ?
    virtual FrameContext& getCurrentFrame() = 0;

    virtual void drawSkybox() = 0;

    Backend backend() const { return m_backend; }

    const RenderCapabilities& getCapabilities() const { return m_capabilities; }

    // @TODO: refactor
    virtual ICommandList* getCommandList() const = 0;

protected:
    virtual Ref<GpuResource> createTextureImpl(const GpuResourceDesc& texture_desc, const SamplerDesc& sampler_desc) = 0;

    virtual void render() = 0;
    virtual void present() = 0;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void moveToNextFrame() = 0;
    virtual Ref<FrameContext> createFrameContext() = 0;

    virtual void onWindowResize(int width, int height) = 0;

protected:
    virtual void updateEmitters(const Scene& scene) = 0;

    RenderCapabilities m_capabilities{};

    const Backend m_backend;
};

}  // namespace cave::render
