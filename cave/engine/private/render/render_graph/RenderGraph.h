#pragma once
#include "cave/core/math/Rect.h"

#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/pixel_format.h"
#include "RenderPass.h"

// clang-format off
namespace cave { struct FrameData; }
// clang-format on

namespace cave::render {

class CompiledGraph;

class RenderGraph {
public:
    struct CreateDesc {
        std::string debug_name;
        GpuTextureDesc resourceDesc;
        SamplerDesc samplerDesc = PointClampSampler();
    };

    struct ImportDesc {
        GpuTextureId external;
    };

    RenderGraph(const math::IntRect& rect)
        : m_viewport(rect) {}

    RenderPass& addRenderPass(std::string_view name);

    [[nodiscard]]
    auto compile() -> Result<Ref<CompiledGraph>>;

    static GpuTextureDesc buildDefaultTextureDesc(PixelFormat format,
                                                  AttachmentType type,
                                                  uint32_t width,
                                                  uint32_t height,
                                                  uint32_t array_size = 1,
                                                  ResourceMiscFlags misc_flag = RESOURCE_MISC_NONE,
                                                  uint32_t mips_level = 0);

    GpuTextureDesc buildDefaultTextureDesc(PixelFormat format,
                                           AttachmentType type,
                                           uint32_t array_size = 1,
                                           ResourceMiscFlags misc_flag = RESOURCE_MISC_NONE,
                                           uint32_t mips_level = 0) {

        return buildDefaultTextureDesc(format,
                                       type,
                                       m_viewport.w,
                                       m_viewport.h,
                                       array_size,
                                       misc_flag,
                                       mips_level);
    }

    RGTextureId createTexture(CreateDesc&& info);
    RGTextureId importTexture(ImportDesc&& info);
    RGDependencyId createDependency();

protected:
    math::IntRect m_viewport;

    Vector<RenderPass> m_passes;

private:
    RGTextureNode* getLogicalTexture(RGTextureId handle);
    const RGTextureNode* getLogicalTexture(RGTextureId handle) const;

    RGTextureId allocHandle();
    Vector<RGTextureNode> m_textures;
    RGTextureId::Type m_handle_id{};
};

}  // namespace cave::render
