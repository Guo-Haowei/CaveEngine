#include "RenderGraph.h"

#include "engine/private/algorithm/algorithm.h"
#include "engine/private/renderer/renderer_misc.h"
#include "engine/private/renderer/sampler.h"
#include "CompiledGraph.h"
#include "RenderGraphDefines.h"
#include "RenderPass.h"

namespace cave::render {

RenderPass& RenderGraph::AddPass(std::string_view p_pass_name) {
    RenderPass builder{ p_pass_name };
    m_passes.push_back(builder);
    return m_passes.back();
}

#define DEBUG_GRAPH_COMPILE NOT_IN_USE
#if USING(DEBUG_GRAPH_COMPILE)
#define DEBUG_PRINT(...) LOG(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

auto RenderGraph::Compile() -> Result<std::shared_ptr<CompiledGraph>> {
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
#if USING(DEBUG_GRAPH_COMPILE)
        std::string_view debug_name;
#endif
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
        const RenderPass& pass = m_passes[i];
        RGPassNode& pass_node = nodes[i];
        pass_node.id = i;
#if USING(DEBUG_GRAPH_COMPILE)
        pass_node.debug_name = pass.GetName();
#endif

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
#if USING(DEBUG_GRAPH_COMPILE)
                return CAVE_ERROR(ErrorCode::ERR_CYCLIC_LINK, "circular dependency found {} {}", a.debug_name, b.debug_name);
#else
                return CAVE_ERROR(ErrorCode::ERR_CYCLIC_LINK);
#endif
            }
            if (a_needs_b) {
                edges.push_back(std::make_pair(b.id, a.id));
            }
            if (b_needs_a) {
                edges.push_back(std::make_pair(a.id, b.id));
            }
        }
    }

    auto res = TopologicalSort(N, edges);
    if (res.is_none()) {
        return CAVE_ERROR(ErrorCode::ERR_CYCLIC_LINK);
    }
    auto sorted = res.unwrap_unchecked();

#if USING(DEBUG_GRAPH_COMPILE)
    DEBUG_PRINT("sorted order:");
    for (int idx : sorted) {
        const auto& pass = m_passes[idx];
        DEBUG_PRINT("  -- pase: {}", pass.GetName());
    }
#endif

    auto compiled_graph = std::make_shared<CompiledGraph>();
    compiled_graph->m_textures = std::move(m_textures);
    compiled_graph->m_order = std::move(sorted);
    compiled_graph->m_passes = std::move(m_passes);

    return compiled_graph;
}

RGTextureId RenderGraph::AllocHandle() {
    return { ++m_id };
}

RGTextureNode* RenderGraph::GetLogicalTexture(RGTextureId p_handle) {
    if (p_handle.IsNull()) return nullptr;
    return &m_textures[p_handle.Underlying() - 1];
}

const RGTextureNode* RenderGraph::GetLogicalTexture(RGTextureId p_handle) const {
    if (p_handle.IsNull()) return nullptr;
    return &m_textures[p_handle.Underlying() - 1];
}

RGTextureId RenderGraph::CreateTexture(CreateDesc&& p_info) {
    RGTextureId handle = AllocHandle();
    m_textures.resize(m_id);
    RGTextureNode& node = m_textures.back();
    node.handle = handle;
    node.desc = p_info.resourceDesc;
    node.sampler = p_info.samplerDesc;
    node.debug_name = std::move(p_info.debug_name);
    return handle;
}

RGTextureId RenderGraph::ImportTexture(ImportDesc&& p_info) {
    DEV_ASSERT(p_info.external);
    RGTextureId handle = AllocHandle();
    m_textures.resize(m_id);
    RGTextureNode& node = m_textures.back();
    node.handle = handle;
    node.external = std::move(p_info.external);
    return handle;
}

///  @TODO: remove this
GpuTextureDesc RenderGraph::BuildDefaultTextureDesc(PixelFormat p_format,
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
