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

    RenderGraph(const math::IntRect& p_rect)
        : m_viewport(p_rect) {}

    RenderPass& AddPass(std::string_view p_name);

    [[nodiscard]] auto Compile() -> Result<std::shared_ptr<CompiledGraph>>;

    static GpuTextureDesc BuildDefaultTextureDesc(PixelFormat p_format,
                                                  AttachmentType p_type,
                                                  uint32_t p_width,
                                                  uint32_t p_height,
                                                  uint32_t p_array_size = 1,
                                                  ResourceMiscFlags p_misc_flag = RESOURCE_MISC_NONE,
                                                  uint32_t p_mips_level = 0);

    GpuTextureDesc BuildDefaultTextureDesc(PixelFormat p_format,
                                           AttachmentType p_type,
                                           uint32_t p_array_size = 1,
                                           ResourceMiscFlags p_misc_flag = RESOURCE_MISC_NONE,
                                           uint32_t p_mips_level = 0) {

        return BuildDefaultTextureDesc(p_format,
                                       p_type,
                                       m_viewport.w,
                                       m_viewport.h,
                                       p_array_size,
                                       p_misc_flag,
                                       p_mips_level);
    }

    RGTextureId CreateTexture(CreateDesc&& p_info);
    RGTextureId ImportTexture(ImportDesc&& p_info);

protected:
    math::IntRect m_viewport;

    std::vector<RenderPass> m_passes;

private:
    RGTextureNode* GetLogicalTexture(RGTextureId p_handle);
    const RGTextureNode* GetLogicalTexture(RGTextureId p_handle) const;

    RGTextureId AllocHandle();
    std::vector<RGTextureNode> m_textures;
    RGTextureId::Type m_id{};
};

}  // namespace cave::render
