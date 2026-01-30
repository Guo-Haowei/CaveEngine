#pragma once
#include "engine/private/renderer/sampler.h"
#include "RenderPass.h"
#include "RenderGraphTypes.h"

// clang-format off
namespace cave { struct GpuTexture; }
// clang-format on

namespace cave::render {

struct RenderGraphResourceCreateInfo {
    GpuTextureDesc resourceDesc;
    SamplerDesc samplerDesc = PointClampSampler();
};

class RenderPassBuilder {
public:
    struct Resource {
        std::string name;
        ResourceAccess access;
    };

    RenderPassBuilder& Create(std::string_view p_name, const RenderGraphResourceCreateInfo& p_desc);
    RenderPassBuilder& Import(std::string_view p_name, ImportFunc&& p_func);

    RenderPassBuilder& Read(ResourceAccess p_access, std::string_view p_name);
    RenderPassBuilder& Write(ResourceAccess p_access, std::string_view p_name);
    RenderPassBuilder& SetExecuteFunc(ExecuteFunc p_func);

    std::string_view GetName() const { return m_name; }

private:
    RenderPassBuilder(std::string_view p_name)
        : m_name{ p_name } {}

    std::string m_name;
    std::vector<std::pair<std::string, RenderGraphResourceCreateInfo>> m_creates;
    std::vector<std::pair<std::string, ImportFunc>> m_imports;
    std::vector<Resource> m_reads;
    std::vector<Resource> m_writes;
    ExecuteFunc m_func;

    friend class RenderGraphBuilder;
};

}  // namespace cave::render
