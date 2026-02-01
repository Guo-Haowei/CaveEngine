#pragma once
#include "engine/private/renderer/sampler.h"
#include "RGRenderPass.h"
#include "RenderGraphTypes.h"

// clang-format off
namespace cave { struct GpuTexture; }
// clang-format on

namespace cave::render {

class RenderGraphBuilder;

struct RGResourceCreateDesc {
    std::string debug_name;
    GpuTextureDesc resourceDesc;
    SamplerDesc samplerDesc = PointClampSampler();
};

struct RGResourceImportDesc {
    std::string debug_name;
    RGImportFunc func;
};

class RenderPassBuilder {
public:
    struct Resource {
        RGTextureId handle;
        ResourceAccess access;
    };

    RenderPassBuilder& Read(ResourceAccess p_access, RGTextureId p_handle);

    RenderPassBuilder& WriteColor(RGTextureId p_handle,
                                  const TextureViewDesc& p_tex_view_desc,
                                  LoadOp p_load = LoadOp::Load);

    RenderPassBuilder& WriteDepth(RGTextureId p_handle,
                                  const TextureViewDesc& p_tex_view_desc,
                                  LoadOp p_depth_load = LoadOp::Load,
                                  float p_clear_depth = 1.0f,
                                  LoadOp p_stencil_load = LoadOp::Load,
                                  uint8_t p_clear_stencil = 0);

    RenderPassBuilder& SetExecuteFunc(ExecuteFunc p_func);

    std::string_view GetName() const { return m_name; }

private:
    RenderPassBuilder(std::string_view p_name)
        : m_name(p_name) {}

    std::string m_name;
    std::vector<Resource> m_reads;
    std::vector<Resource> m_writes;
    std::vector<ColorAttachmentDesc> m_colors;
    std::optional<DepthAttachmentDesc> m_depth;

    ExecuteFunc m_func;

    friend class RenderGraphBuilder;
};

}  // namespace cave::render
