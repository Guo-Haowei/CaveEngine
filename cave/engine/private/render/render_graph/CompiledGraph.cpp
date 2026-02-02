#include "CompiledGraph.h"

#include "engine/private/render/render_device/RenderDevice.h"

namespace cave::render {

void CompiledGraph::AddResource(RGTextureId p_handle, const std::shared_ptr<GpuTexture>& p_resource) {
    const int idx = static_cast<int>(m_resources.size());
    m_resources.push_back(p_resource);
    m_resourceLookup.insert({ p_handle, idx });
}

std::shared_ptr<GpuTexture> CompiledGraph::FindResource(RGTextureId p_handle) {
    auto it = m_resourceLookup.find(p_handle);
    if (it == m_resourceLookup.end()) {
        return nullptr;
    }

    return m_resources[it->second];
}

void CompiledGraph::Resolve(IRenderDevice& p_device) {
    // 1. Create/Import resources
    for (const RGTextureNode& node : m_textures) {
        if (node.external) {
            AddResource(node.handle, std::move(node.external));
            continue;
        }

        GpuTextureDesc desc = node.desc;
        // @TODO: get rid of the name
        desc.name = node.debug_name;
        ResourceAccess access = node.access_mask;
        if ((access & ResourceAccess::RTV) != ResourceAccess::NONE) {
            desc.bindFlags |= BIND_RENDER_TARGET;
        }
        if ((access & ResourceAccess::DSV) != ResourceAccess::NONE) {
            desc.bindFlags |= BIND_DEPTH_STENCIL;
        }
        if ((access & ResourceAccess::SRV) != ResourceAccess::NONE) {
            desc.bindFlags |= BIND_SHADER_RESOURCE;
        }
        if ((access & ResourceAccess::UAV) != ResourceAccess::NONE) {
            desc.bindFlags |= BIND_UNORDERED_ACCESS;
        }

        auto texture = p_device.CreateTexture(desc, node.sampler);
        AddResource(node.handle, texture);
    }

    m_compiled_pass.resize(m_order.size());
    int pass_idx = 0;

    for (int idx : m_order) {
        auto& pass_builder = m_passes[idx];

        std::vector<GpuTextureId> srvs;
        std::vector<GpuTextureId> uavs;

        size_t color_idx = 0;
        for (const auto& read : pass_builder.m_reads) {
            if ((bool)(read.access & ResourceAccess::DSV)) {
                GpuTextureId depth_id = FindResource(read.handle);
                pass_builder.m_depth->tex = depth_id;
                break;
            }
        }
        for (const auto& write : pass_builder.m_writes) {
            switch (write.access) {
                case ResourceAccess::DSV: {
                    GpuTextureId depth_id = FindResource(write.handle);
                    DEV_ASSERT(pass_builder.m_depth.has_value() && !pass_builder.m_depth->tex);
                    pass_builder.m_depth->tex = depth_id;
                } break;
                case ResourceAccess::RTV: {
                    GpuTextureId color_id = FindResource(write.handle);
                    DEV_ASSERT(color_idx < pass_builder.m_colors.size());
                    ColorAttachmentDesc& color = pass_builder.m_colors[color_idx];
                    DEV_ASSERT(!color.tex && color_id);
                    color.tex = color_id;
                    ++color_idx;
                } break;
                default:
                    break;
            }
        }

        for (const auto& read : pass_builder.m_reads) {
            switch (read.access) {
                case ResourceAccess::SRV: {
                    auto srv = FindResource(read.handle);
                    srvs.emplace_back(srv);
                } break;
                case ResourceAccess::UAV: {
                    auto uav = FindResource(read.handle);
                    DEV_ASSERT(uav);
                    uavs.emplace_back(uav);
                } break;
                default:
                    break;
            }
        }

        CompiledPass& compiled_pass = m_compiled_pass[pass_idx++];
        compiled_pass.name = std::move(pass_builder.m_name);
        compiled_pass.func = std::move(pass_builder.m_func);

        compiled_pass.srvs = std::move(srvs);
        compiled_pass.uavs = std::move(uavs);
        compiled_pass.colors = std::move(pass_builder.m_colors);
        compiled_pass.depth = std::move(pass_builder.m_depth);
        compiled_pass.viewport = pass_builder.m_viewport;
    }

    // clear caches
    m_textures.clear();
    m_passes.clear();
    m_order.clear();
    m_resources.clear();
    m_resourceLookup.clear();
}

}  // namespace cave::render
