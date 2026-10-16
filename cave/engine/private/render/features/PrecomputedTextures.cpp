#include "PrecomputedTextures.h"

#include "cave/render/rhi/Sampler.h"

// @TODO: rename
#include "engine/private/render/features/LTCMatrix.h"
#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave::render {

constexpr const char RG_RES_LTC1[] = "r:ltc1";
constexpr const char RG_RES_LTC2[] = "r:ltc2";

static GpuTextureId CreateLTC(IRenderDevice& p_device,
                              std::string_view p_name,
                              const float* p_matrix_table) {
    constexpr int LTC_SIZE = 64;
    GpuTextureDesc desc{
        .type = AttachmentType::NONE,
        .dimension = Dimension::Texture2D,
        .width = LTC_SIZE,
        .height = LTC_SIZE,
        .depth = 1,
        .mip_levels = 1,
        .array_size = 1,
        .format = PixelFormat::R32G32B32A32_FLOAT,
        .bind_flags = BIND_SHADER_RESOURCE,
        .misc_flags = RESOURCE_MISC_NONE,
        .initial_data = p_matrix_table,
        .name = std::string(p_name),
    };

    return p_device.createTexture(desc, PointClampSampler());
}

GpuTextureId CreateLTC1(IRenderDevice& p_device) {
    return CreateLTC(p_device, RG_RES_LTC1, LTC1);
}

GpuTextureId CreateLTC2(IRenderDevice& p_device) {
    return CreateLTC(p_device, RG_RES_LTC2, LTC2);
}

}  // namespace cave::render
