#include "RenderGraph.h"

#include "cave/core/algorithm/Graph.h"

#include "CompiledGraph.h"
#include "RenderGraphDefines.h"
#include "RenderPass.h"

// @TODO: refactor
#include "engine/private/renderer/renderer_misc.h"
#include "engine/private/renderer/sampler.h"

namespace cave::render {

#define DEBUG_GRAPH_COMPILE NOT_IN_USE
#if USING(DEBUG_GRAPH_COMPILE)
#define DEBUG_PRINT(...) LOG_INFO(LogChannel::Render, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

RenderPass& RenderGraph::addRenderPass(std::string_view pass_name) {
    RenderPass builder{ pass_name };
    m_passes.push_back(builder);
    return m_passes.back();
}

auto RenderGraph::compile() -> Result<Ref<CompiledGraph>> {
#if USING(DEBUG_GRAPH_COMPILE)
    for (const auto& pass : m_passes) {
        DEBUG_PRINT("found pass: {}", pass.name());

        auto helper = [this](const char* p_string, std::span<const RenderPass::Resource> p_resources) {
            if (p_resources.empty()) {
                return;
            }
            DEBUG_PRINT("  {}", p_string);
            for (const auto& res : p_resources) {
                const RGTextureNode* node = getLogicalTexture(res.handle);
                if (!node) continue;
                std::string access;
                if ((bool)(res.access & ResourceAccess::SRV)) access += " SRV |";
                if ((bool)(res.access & ResourceAccess::RTV)) access += " RTV |";
                if ((bool)(res.access & ResourceAccess::DSV)) access += " DSV |";
                if ((bool)(res.access & ResourceAccess::UAV)) access += " UAV |";
                if (!access.empty()) access.pop_back();

                DEBUG_PRINT("     {}(id: {}) {}",
                            node->debug_name,
                            res.handle.underlying(),
                            access);
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
        int id = 0;
#if USING(DEBUG_GRAPH_COMPILE)
        std::string_view debug_name;
#endif
        Vector<RGTextureId> in_nodes;
        HashSet<RGTextureId> out_nodes;

        bool requres(const RGPassNode& other) const {
            for (RGTextureId in : in_nodes) {
                if (other.out_nodes.find(in) != other.out_nodes.end()) {
                    return true;
                }
            }
            return false;
        }
    };
    Vector<RGPassNode> nodes;
    nodes.resize(N);

    for (int i = 0; i < N; ++i) {
        const RenderPass& pass = m_passes[i];
        RGPassNode& pass_node = nodes[i];
        pass_node.id = i;
#if USING(DEBUG_GRAPH_COMPILE)
        pass_node.debug_name = pass.name();
#endif

        for (const auto& read : pass.m_reads) {
            RGTextureNode* tex = getLogicalTexture(read.handle);
            if (!tex) continue;
            pass_node.in_nodes.push_back(read.handle);
            tex->access_mask |= read.access;
        }
        for (const auto& write : pass.m_writes) {
            RGTextureNode* tex = getLogicalTexture(write.handle);
            if (!tex) continue;
            pass_node.out_nodes.insert(write.handle);
            tex->access_mask |= write.access;
        }
    }

    Vector<std::pair<int, int>> edges;
    for (int i = 0; i < N - 1; ++i) {
        for (int j = i + 1; j < N; ++j) {
            const RGPassNode& a = nodes[i];
            const RGPassNode& b = nodes[j];
            const bool a_needs_b = a.requres(b);
            const bool b_needs_a = b.requres(a);
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
        DEBUG_PRINT("  -- pass: {}", pass.name());
    }
#endif

    auto compiled_graph = MakeRef<CompiledGraph>();
    compiled_graph->m_textures = std::move(m_textures);
    compiled_graph->m_order = std::move(sorted);
    compiled_graph->m_passes = std::move(m_passes);

    return compiled_graph;
}

RGTextureId RenderGraph::allocHandle() {
    return { ++m_handle_id };
}

RGTextureNode* RenderGraph::getLogicalTexture(RGTextureId handle) {
    if (handle.isNull()) return nullptr;
    return &m_textures[handle.underlying() - 1];
}

const RGTextureNode* RenderGraph::getLogicalTexture(RGTextureId handle) const {
    if (handle.isNull()) return nullptr;
    return &m_textures[handle.underlying() - 1];
}

RGTextureId RenderGraph::createTexture(CreateDesc&& info) {
    RGTextureId handle = allocHandle();
    m_textures.resize(m_handle_id);
    RGTextureNode& node = m_textures.back();
    node.handle = handle;
    node.desc = info.resourceDesc;
    node.sampler = info.samplerDesc;
    node.debug_name = std::move(info.debug_name);
    return handle;
}

RGTextureId RenderGraph::importTexture(ImportDesc&& info) {
    DEV_ASSERT(info.external);
    RGTextureId handle = allocHandle();
    m_textures.resize(m_handle_id);
    RGTextureNode& node = m_textures.back();
    node.handle = handle;
    node.external = std::move(info.external);
    return handle;
}

RGDependencyId RenderGraph::createDependency() {
    RGTextureId handle = allocHandle();
    m_textures.resize(m_handle_id);
    RGTextureNode& node = m_textures.back();
    node.is_dependency = true;
    return RGDependencyId(handle);
}

///  @TODO: remove this
GpuTextureDesc RenderGraph::buildDefaultTextureDesc(PixelFormat format,
                                                    AttachmentType type,
                                                    uint32_t width,
                                                    uint32_t height,
                                                    uint32_t array_size,
                                                    ResourceMiscFlags misc_flag,
                                                    uint32_t mips_level) {
    GpuTextureDesc desc{};
    desc.type = type;
    desc.format = format;
    desc.arraySize = array_size;
    desc.dimension = Dimension::TEXTURE_2D;
    desc.width = width;
    desc.height = height;
    desc.mipLevels = mips_level ? mips_level : 1;
    desc.miscFlags = misc_flag;
    desc.initialData = nullptr;

    switch (type) {
        case AttachmentType::COLOR_2D:
        case AttachmentType::DEPTH_2D:
        case AttachmentType::DEPTH_STENCIL_2D:
        case AttachmentType::SHADOW_2D:
            desc.dimension = Dimension::TEXTURE_2D;
            break;
        case AttachmentType::COLOR_CUBE:
            desc.dimension = Dimension::TEXTURE_CUBE;
            desc.miscFlags |= RESOURCE_MISC_TEXTURECUBE;
            DEV_ASSERT(array_size == 6);
            break;
        case AttachmentType::SHADOW_CUBE_ARRAY:
            desc.dimension = Dimension::TEXTURE_CUBE_ARRAY;
            desc.miscFlags |= RESOURCE_MISC_TEXTURECUBE;
            DEV_ASSERT(array_size / 6 > 0);
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
