#include "RenderPassBuilder.h"
#include "RenderGraphBuilder.h"

namespace cave::render {

RenderPassBuilder& RenderPassBuilder::Read(ResourceAccess p_access, RGTextureId p_handle) {
    m_reads.emplace_back(Resource{ p_handle, p_access });
    return *this;
}

RenderPassBuilder& RenderPassBuilder::WriteColor(RGTextureId p_handle,
                                                 const TextureViewDesc& p_tex_view_desc,
                                                 LoadOp p_load) {
    m_writes.emplace_back(Resource{ p_handle, ResourceAccess::RTV });

    ColorAttachmentDesc desc{
        .tex = nullptr,
        .view = p_tex_view_desc,
        .load = p_load,
        .clear_color = { 0, 0, 0, 0 },
    };
    m_colors.push_back(desc);

    return *this;
}

RenderPassBuilder& RenderPassBuilder::WriteDepth(RGTextureId p_handle,
                                                 const TextureViewDesc& p_tex_view_desc,
                                                 LoadOp p_depth_load,
                                                 float p_clear_depth,
                                                 LoadOp p_stencil_load,
                                                 uint8_t p_clear_stencil) {
    m_writes.emplace_back(Resource{ p_handle, ResourceAccess::DSV });

    if (DEV_VERIFY(!m_depth)) {
        DepthAttachmentDesc desc{
            .tex = nullptr,
            .view = p_tex_view_desc,
            .depth_load = p_depth_load,
            .clear_depth = p_clear_depth,
            .stencil_load = p_stencil_load,
            .clear_stencil = p_clear_stencil,
        };
        m_depth = desc;
    }

    return *this;
}

RenderPassBuilder& RenderPassBuilder::SetExecuteFunc(ExecuteFunc p_func) {
    m_func = p_func;
    return *this;
}

}  // namespace cave::render
