#pragma once
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/pixel_format.h"
#include "RenderPassBuilder.h"

// clang-format off
namespace cave { struct FrameData; }
namespace cave { class IRenderDevice; }
// clang-format on

namespace cave::render {

class RenderGraph;

struct RenderGraphBuilderConfig {
    bool enablePointShadow = true;
    bool enableVxgi = true;
    bool enableIbl = true;
    bool enableBloom = true;
    bool enableHighlight = true;

    int frameWidth;
    int frameHeight;
};

class RenderGraphBuilder {
public:
    RenderGraphBuilder(const RenderGraphBuilderConfig& p_config);

    RenderPassBuilder& AddPass(std::string_view p_name);

    [[nodiscard]] auto Compile() -> Result<std::shared_ptr<RenderGraph>>;

    GpuTextureDesc BuildDefaultTextureDesc(PixelFormat p_format,
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
                                       m_config.frameWidth,
                                       m_config.frameHeight,
                                       p_array_size,
                                       p_misc_flag,
                                       p_mips_level);
    }

    RGTextureId CreateTexture(RGResourceCreateDesc&& p_info);
    RGTextureId ImportTexture(RGResourceImportDesc&& p_info);

protected:
    RenderGraphBuilderConfig m_config;
    IRenderDevice& m_graphicsManager;

    std::vector<RenderPassBuilder> m_passes;

private:
    RGTextureNode* GetLogicalTexture(RGTextureId p_handle);
    const RGTextureNode* GetLogicalTexture(RGTextureId p_handle) const;

    RGTextureId AllocHandle();
    std::vector<RGTextureNode> m_textures;
    RGTextureId::Type m_id{};
};

}  // namespace cave::render
