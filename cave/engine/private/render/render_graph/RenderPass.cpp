#include "RenderPass.h"
#include "RenderGraph.h"

namespace cave::render {

RenderPass& RenderPass::writeDependency(RGDependencyId id) {
    if (id.isNull()) {
        return *this;
    }

    m_writes.emplace_back(id, ResourceAccess::None);
    return *this;
}

RenderPass& RenderPass::readDependency(RGDependencyId id) {
    if (id.isNull()) {
        return *this;
    }

    return read(ResourceAccess::None, id);
}

RenderPass& RenderPass::read(ResourceAccess access, RGTextureId id) {
    m_reads.emplace_back(id, access);
    return *this;
}

RenderPass& RenderPass::writeColor(RGTextureId id,
                                   const TextureViewDesc& tex_view_desc,
                                   LoadOp load) {
    m_writes.emplace_back(id, ResourceAccess::RTV);

    ColorAttachmentDesc desc{
        .tex = nullptr,
        .view = tex_view_desc,
        .load = load,
        .clear_color = { 0, 0, 0, 0 },
    };
    m_colors.push_back(desc);

    return *this;
}

RenderPass& RenderPass::readOrWriteDepth(Vector<Resource>& array,
                                         RGTextureId id,
                                         const TextureViewDesc& tex_view_desc,
                                         LoadOp depth_load,
                                         float clear_depth,
                                         LoadOp stencil_load,
                                         uint8_t clear_stencil) {
    if (id.isNull()) {
        return *this;
    }

    array.emplace_back(id, ResourceAccess::DSV);

    if (DEV_VERIFY(!m_depth)) {
        DepthAttachmentDesc desc{
            .tex = nullptr,
            .view = tex_view_desc,
            .depth_load = depth_load,
            .clear_depth = clear_depth,
            .stencil_load = stencil_load,
            .clear_stencil = clear_stencil,
        };
        m_depth = desc;
    }

    return *this;
}

}  // namespace cave::render
