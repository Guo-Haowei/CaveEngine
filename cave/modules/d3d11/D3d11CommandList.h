#pragma once
#include <d3d11_1.h>
#include <wrl/client.h>

#include "cave/render/rhi/ICommandList.h"

namespace cave::render {

class D3d11PipelineStateManager;
class D3d11ViewCache;

class D3d11CommandList final : public ICommandList {
public:
    explicit D3d11CommandList(D3d11ViewCache& view_cache,
                              D3d11PipelineStateManager& pso_manager,
                              Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
                              Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> annotation)
        : m_view_cache(view_cache)
        , m_pipeline_state_manager(pso_manager)
        , m_device_context(context)
        , m_annotation(annotation) {}

    void draw(uint32_t count,
              uint32_t offset) override;

    void drawInstanced(uint32_t instance_count,
                       uint32_t count,
                       uint32_t offset) override;

    void drawIndexed(uint32_t count,
                     uint32_t offset) override;

    void drawIndexedInstanced(uint32_t instance_count,
                              uint32_t count,
                              uint32_t offset) override;

    void beginRendering(const RenderingInfo& info) override;
    void endRendering() override;

    void pushDebugGroup(std::string_view name) override;
    void popDebugGroup() override;

    void setViewport(const Viewport& viewport) override;

    void setPipelineState(PipelineStateName name) override;

    void setStencilRef(uint32_t ref) override;

    void bindConstantBufferRange(const GpuBuffer* buffer,
                                 uint32_t size,
                                 uint32_t offset) override;

    void dispatch(uint32_t num_groups_x,
                  uint32_t num_groups_y,
                  uint32_t num_groups_z) override;

    // @TODO: remove these
    void setMesh(const GpuMesh* mesh) override;

    void bindTexture(Dimension dimension, uint64_t handle, int slot) override;
    void unbindTexture(Dimension dimension, int slot) override;

    void bindStructuredBuffer(int slot, const GpuBuffer* buffer) override;
    void unbindStructuredBuffer(int slot) override;
    void bindStructuredBufferSRV(int slot, const GpuBuffer* buffer) override;
    void unbindStructuredBufferSRV(int slot) override;

    void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) override;
    void unbindUnorderedAccessView(uint32_t slot) override;

private:
    D3d11ViewCache& m_view_cache;
    D3d11PipelineStateManager& m_pipeline_state_manager;

    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_device_context;
    Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_annotation;

    struct {
        ID3D11RasterizerState* rasterizer = nullptr;
        ID3D11DepthStencilState* depthStencil = nullptr;
        uint32_t stencilRef = 0xFFFFFFFF;
        ID3D11BlendState* blendState = nullptr;
    } m_state_cache;
};

}  // namespace cave::render
