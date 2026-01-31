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
    RenderPassBuilder& Write(ResourceAccess p_access, RGTextureId p_handle);
    RenderPassBuilder& SetExecuteFunc(ExecuteFunc p_func);

    std::string_view GetName() const { return m_name; }

private:
    RenderPassBuilder(std::string_view p_name)
        : m_name(p_name) {}

    std::string m_name;
    std::vector<Resource> m_reads;
    std::vector<Resource> m_writes;
    ExecuteFunc m_func;

    friend class RenderGraphBuilder;
};

}  // namespace cave::render
