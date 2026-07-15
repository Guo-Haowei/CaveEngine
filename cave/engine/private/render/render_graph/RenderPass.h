#pragma once
#include "engine/private/renderer/sampler.h"
#include "CompiledPass.h"
#include "RenderGraphTypes.h"

namespace cave::render {

class RenderPass {
public:
    struct Resource {
        RGTextureId handle;
        ResourceAccess access;
    };

    RenderPass(std::string_view name)
        : m_name(name) {}

    RenderPass& read(ResourceAccess access, RGTextureId handle);

    RenderPass& readDepth(RGTextureId handle,
                          const TextureViewDesc& tex_view_desc,
                          LoadOp depth_load = LoadOp::Load,
                          float clear_depth = 1.0f,
                          LoadOp stencil_load = LoadOp::Load,
                          uint8_t clear_stencil = 0) {

        return readOrWriteDepth(m_reads,
                                handle,
                                tex_view_desc,
                                depth_load,
                                clear_depth,
                                stencil_load,
                                clear_stencil);
    }

    RenderPass& writeDependency(RGDependencyId id);
    RenderPass& readDependency(RGDependencyId id);

    RenderPass& writeColor(RGTextureId handle,
                           const TextureViewDesc& tex_view_desc,
                           LoadOp load = LoadOp::Load);

    RenderPass& writeDepth(RGTextureId handle,
                           const TextureViewDesc& tex_view_desc,
                           LoadOp depth_load = LoadOp::Load,
                           float clear_depth = 1.0f,
                           LoadOp stencil_load = LoadOp::Load,
                           uint8_t clear_stencil = 0) {
        return readOrWriteDepth(m_writes,
                                handle,
                                tex_view_desc,
                                depth_load,
                                clear_depth,
                                stencil_load,
                                clear_stencil);
    }

    RenderPass& setExecuteFunc(ExecuteFunc func) {
        m_func = std::move(func);
        return *this;
    }

    RenderPass& setViewport(const Viewport& viewport) {
        m_viewport = viewport;
        return *this;
    }

    std::string_view name() const { return m_name; }

private:
    RenderPass& readOrWriteDepth(std::vector<Resource>& array,
                                 RGTextureId handle,
                                 const TextureViewDesc& tex_view_desc,
                                 LoadOp depth_load,
                                 float clear_depth,
                                 LoadOp stencil_load,
                                 uint8_t clear_stencil);

    String m_name;
    Vector<Resource> m_reads;
    Vector<Resource> m_writes;
    Vector<ColorAttachmentDesc> m_colors;
    std::optional<DepthAttachmentDesc> m_depth;
    std::optional<Viewport> m_viewport;

    ExecuteFunc m_func;

    friend class RenderGraph;
    friend class CompiledGraph;
};

}  // namespace cave::render
