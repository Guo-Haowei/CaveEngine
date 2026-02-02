#pragma once
#include "engine/private/renderer/sampler.h"
#include "RenderPass.h"
#include "RenderGraphTypes.h"

namespace cave::render {

class RenderPassBuilder {
public:
    struct Resource {
        RGTextureId handle;
        ResourceAccess access;
    };

    RenderPassBuilder& Read(ResourceAccess p_access, RGTextureId p_handle);

    RenderPassBuilder& ReadDepth(RGTextureId p_handle,
                                 const TextureViewDesc& p_tex_view_desc,
                                 LoadOp p_depth_load = LoadOp::Load,
                                 float p_clear_depth = 1.0f,
                                 LoadOp p_stencil_load = LoadOp::Load,
                                 uint8_t p_clear_stencil = 0) {

        return ReadOrWriteDepth(m_reads,
                                p_handle,
                                p_tex_view_desc,
                                p_depth_load,
                                p_clear_depth,
                                p_stencil_load,
                                p_clear_stencil);
    }

    RenderPassBuilder& WriteColor(RGTextureId p_handle,
                                  const TextureViewDesc& p_tex_view_desc,
                                  LoadOp p_load = LoadOp::Load);

    RenderPassBuilder& WriteDepth(RGTextureId p_handle,
                                  const TextureViewDesc& p_tex_view_desc,
                                  LoadOp p_depth_load = LoadOp::Load,
                                  float p_clear_depth = 1.0f,
                                  LoadOp p_stencil_load = LoadOp::Load,
                                  uint8_t p_clear_stencil = 0) {

        return ReadOrWriteDepth(m_writes,
                                p_handle,
                                p_tex_view_desc,
                                p_depth_load,
                                p_clear_depth,
                                p_stencil_load,
                                p_clear_stencil);
    }

    RenderPassBuilder& SetExecuteFunc(ExecuteFunc p_func) {
        m_func = std::move(p_func);
        return *this;
    }

    RenderPassBuilder& SetViewport(const Viewport& p_viewport) {
        m_viewport = p_viewport;
        return *this;
    }

    std::string_view GetName() const { return m_name; }

private:
    RenderPassBuilder(std::string_view p_name)
        : m_name(p_name) {}

    RenderPassBuilder& ReadOrWriteDepth(std::vector<Resource>& p_array,
                                        RGTextureId p_handle,
                                        const TextureViewDesc& p_tex_view_desc,
                                        LoadOp p_depth_load,
                                        float p_clear_depth,
                                        LoadOp p_stencil_load,
                                        uint8_t p_clear_stencil);

    std::string m_name;
    std::vector<Resource> m_reads;
    std::vector<Resource> m_writes;
    std::vector<ColorAttachmentDesc> m_colors;
    std::optional<DepthAttachmentDesc> m_depth;
    std::optional<Viewport> m_viewport;

    ExecuteFunc m_func;

    friend class RenderGraphBuilder;
};

}  // namespace cave::render
