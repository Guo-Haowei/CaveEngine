#include "RenderGraphBuilder.h"

#include "engine/private/algorithm/algorithm.h"
#include "engine/private/renderer/renderer_misc.h"
#include "engine/private/renderer/sampler.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "RenderGraph.h"
#include "RenderGraphDefines.h"
#include "RenderPassBuilder.h"

namespace cave::render {

RenderGraphBuilder::RenderGraphBuilder(const RenderGraphBuilderConfig& p_config)
    : m_config(p_config), m_graphicsManager(RenderDevice::GetSingleton()) {
}

RenderPassBuilder& RenderGraphBuilder::AddPass(std::string_view p_pass_name) {
    RenderPassBuilder builder{ p_pass_name };
    m_passes.push_back(builder);
    return m_passes.back();
}

#define DEBUG_GRAPH_COMPILE IN_USE
#if USING(DEBUG_GRAPH_COMPILE)
#define DEBUG_PRINT(...) LOG(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

auto RenderGraphBuilder::Compile() -> Result<std::shared_ptr<RenderGraph>> {
#if USING(DEBUG_GRAPH_COMPILE)
    for (const auto& pass : m_passes) {
        DEBUG_PRINT("found pass: {}", pass.GetName());

        auto helper = [this](const char* p_string, std::span<const RenderPassBuilder::Resource> p_resources) {
            if (p_resources.empty()) return;
            DEBUG_PRINT("  {}", p_string);
            for (const auto& res : p_resources) {
                const RGTextureNode* node = GetLogicalTexture(res.handle);
                if (!node) continue;
                std::string access;
                if ((bool)(res.access & ResourceAccess::SRV)) access += " SRV |";
                if ((bool)(res.access & ResourceAccess::RTV)) access += " RTV |";
                if ((bool)(res.access & ResourceAccess::DSV)) access += " DSV |";
                if ((bool)(res.access & ResourceAccess::UAV)) access += " UAV |";
                if (!access.empty()) access.pop_back();

                DEBUG_PRINT("     {}(id: {}) {}",
                            node->debug_name,
                            res.handle.Underlying(), access);
            }
        };
        helper("in: ", pass.m_reads);
        helper("out: ", pass.m_writes);
    }
#endif
    const int N = static_cast<int>(m_passes.size());
    if (N == 0) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "no pass registered");
    }

    struct RGPassNode {
        int id;
        std::string_view debug_name;
        std::vector<RGTextureId> in_nodes;
        std::unordered_set<RGTextureId> out_nodes;

        bool Needs(const RGPassNode& p_other) const {
            for (RGTextureId in : in_nodes)
                if (p_other.out_nodes.find(in) != p_other.out_nodes.end())
                    return true;
            return false;
        }
    };
    std::vector<RGPassNode> nodes;
    nodes.resize(N);

    for (int i = 0; i < N; ++i) {
        const RenderPassBuilder& pass = m_passes[i];
        RGPassNode& pass_node = nodes[i];
        pass_node.id = i;
        pass_node.debug_name = pass.GetName();

        for (const auto& read : pass.m_reads) {
            RGTextureNode* tex = GetLogicalTexture(read.handle);
            if (!tex) continue;
            pass_node.in_nodes.push_back(read.handle);
            tex->access_mask |= read.access;
        }
        for (const auto& write : pass.m_writes) {
            RGTextureNode* tex = GetLogicalTexture(write.handle);
            if (!tex) continue;
            pass_node.out_nodes.insert(write.handle);
            tex->access_mask |= write.access;
        }
    }

    std::vector<std::pair<int, int>> edges;
    for (int i = 0; i < N - 1; ++i) {
        for (int j = i + 1; j < N; ++j) {
            const RGPassNode& a = nodes[i];
            const RGPassNode& b = nodes[j];
            const bool a_needs_b = a.Needs(b);
            const bool b_needs_a = b.Needs(a);
            if (a_needs_b && b_needs_a) {
                return CAVE_ERROR(ErrorCode::ERR_CYCLIC_LINK, "circular dependency found {} {}", a.debug_name, b.debug_name);
            }
            if (a_needs_b) {
                edges.push_back(std::make_pair(b.id, a.id));
            }
            if (b_needs_a) {
                edges.push_back(std::make_pair(a.id, b.id));
            }
        }
    }

#if USING(DEBUG_GRAPH_COMPILE)
    for (const auto& edge : edges) {
        const RGPassNode& from = nodes[edge.first];
        const RGPassNode& to = nodes[edge.second];
        DEBUG_PRINT("found edge from {} to {}", from.debug_name, to.debug_name);
    }
#endif

    auto res = TopologicalSort(N, edges);
    if (res.is_none()) {
        return CAVE_ERROR(ErrorCode::ERR_CYCLIC_LINK);
    }
    auto sorted = res.unwrap_unchecked();

    auto render_graph = std::make_shared<RenderGraph>();

    // @TODO: move texture and framebuffer creation outside Compile

    // 1. Create/Import resources
    for (const RGTextureNode& node : m_textures) {
        if (node.import_fn) {
            auto texture = node.import_fn();
            render_graph->AddResource(node.handle, std::move(texture));
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

        auto texture = m_graphicsManager.CreateTexture(desc, node.sampler);
        render_graph->AddResource(node.handle, texture);
    }

    // 2. Create framebuffer (should only create it for opengl)
    for (int idx : sorted) {
        auto& pass_builder = m_passes[idx];

        std::vector<GpuTextureId> srvs;
        std::vector<GpuTextureId> uavs;

        size_t color_idx = 0;
        for (const auto& write : pass_builder.m_writes) {
            switch (write.access) {
                case ResourceAccess::DSV: {
                    GpuTextureId depth_id = render_graph->FindResource(write.handle);
                    DEV_ASSERT(pass_builder.m_depth.has_value() && !pass_builder.m_depth->tex);
                    pass_builder.m_depth->tex = depth_id;
                } break;
                case ResourceAccess::RTV: {
                    GpuTextureId color_id = render_graph->FindResource(write.handle);
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
                    auto srv = render_graph->FindResource(read.handle);
                    srvs.emplace_back(srv);
                } break;
                case ResourceAccess::UAV: {
                    auto uav = render_graph->FindResource(read.handle);
                    DEV_ASSERT(uav);
                    uavs.emplace_back(uav);
                } break;
                default:
                    break;
            }
        }

        auto render_pass = std::make_shared<RGRenderPass>();
        render_pass->name = std::move(pass_builder.m_name);
        render_pass->func = std::move(pass_builder.m_func);

        render_pass->srvs = std::move(srvs);
        render_pass->uavs = std::move(uavs);
        render_pass->colors = std::move(pass_builder.m_colors);
        render_pass->depth = std::move(pass_builder.m_depth);
        render_pass->viewport = pass_builder.m_viewport;

        render_graph->AddPass(pass_builder.m_name, render_pass);
    }

    return Result<std::shared_ptr<RenderGraph>>(render_graph);
}

RGTextureId RenderGraphBuilder::AllocHandle() {
    return { ++m_id };
}

RGTextureNode* RenderGraphBuilder::GetLogicalTexture(RGTextureId p_handle) {
    if (p_handle.IsNull()) return nullptr;
    return &m_textures[p_handle.Underlying() - 1];
}

const RGTextureNode* RenderGraphBuilder::GetLogicalTexture(RGTextureId p_handle) const {
    if (p_handle.IsNull()) return nullptr;
    return &m_textures[p_handle.Underlying() - 1];
}

RGTextureId RenderGraphBuilder::CreateTexture(RGResourceCreateDesc&& p_info) {
    RGTextureId handle = AllocHandle();
    m_textures.resize(m_id);
    RGTextureNode& node = m_textures.back();
    node.handle = handle;
    node.desc = p_info.resourceDesc;
    node.sampler = p_info.samplerDesc;
    node.debug_name = std::move(p_info.debug_name);
    return handle;
}

RGTextureId RenderGraphBuilder::ImportTexture(RGResourceImportDesc&& p_info) {
    RGTextureId handle = AllocHandle();
    m_textures.resize(m_id);
    RGTextureNode& node = m_textures.back();
    node.handle = handle;
    node.import_fn = std::move(p_info.func);
    node.debug_name = std::move(p_info.debug_name);
    return handle;
}

///  @TODO: remove this
GpuTextureDesc RenderGraphBuilder::BuildDefaultTextureDesc(PixelFormat p_format,
                                                           AttachmentType p_type,
                                                           uint32_t p_width,
                                                           uint32_t p_height,
                                                           uint32_t p_array_size,
                                                           ResourceMiscFlags p_misc_flag,
                                                           uint32_t p_mips_level) {
    GpuTextureDesc desc{};
    desc.type = p_type;
    desc.format = p_format;
    desc.arraySize = p_array_size;
    desc.dimension = Dimension::TEXTURE_2D;
    desc.width = p_width;
    desc.height = p_height;
    desc.mipLevels = p_mips_level ? p_mips_level : 1;
    desc.miscFlags = p_misc_flag;
    desc.initialData = nullptr;

    switch (p_type) {
        case AttachmentType::COLOR_2D:
        case AttachmentType::DEPTH_2D:
        case AttachmentType::DEPTH_STENCIL_2D:
        case AttachmentType::SHADOW_2D:
            desc.dimension = Dimension::TEXTURE_2D;
            break;
        case AttachmentType::COLOR_CUBE:
            desc.dimension = Dimension::TEXTURE_CUBE;
            desc.miscFlags |= RESOURCE_MISC_TEXTURECUBE;
            DEV_ASSERT(p_array_size == 6);
            break;
        case AttachmentType::SHADOW_CUBE_ARRAY:
            desc.dimension = Dimension::TEXTURE_CUBE_ARRAY;
            desc.miscFlags |= RESOURCE_MISC_TEXTURECUBE;
            DEV_ASSERT(p_array_size / 6 > 0);
            break;
        case AttachmentType::RW_TEXTURE:
            break;
        default:
            CRASH_NOW();
            break;
    }
    return desc;
};

}  // namespace cave::render
