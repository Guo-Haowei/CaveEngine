#include "PrecomputedTextures.h"

#include "engine/private/render/features/LTCMatrix.h"
#include "engine/private/runtime/framework/IRenderDevice.h"

// @TODO: rename
#include "engine/private/renderer/sampler.h"

namespace cave::render {

constexpr const char RG_RES_LTC1[] = "r:ltc1";
constexpr const char RG_RES_LTC2[] = "r:ltc2";

static GpuTextureId CreateLTC(IRenderDevice& p_device,
                              std::string_view p_name,
                              const float* p_matrix_table) {
    constexpr int LTC_SIZE = 64;
    GpuTextureDesc desc{
        .type = AttachmentType::NONE,
        .dimension = Dimension::TEXTURE_2D,
        .width = LTC_SIZE,
        .height = LTC_SIZE,
        .depth = 1,
        .mipLevels = 1,
        .arraySize = 1,
        .format = PixelFormat::R32G32B32A32_FLOAT,
        .bindFlags = BIND_SHADER_RESOURCE,
        .miscFlags = RESOURCE_MISC_NONE,
        .initialData = p_matrix_table,
        .name = std::string(p_name),
    };

    return p_device.CreateTexture(desc, PointClampSampler());
}

GpuTextureId CreateLTC1(IRenderDevice& p_device) {
    return CreateLTC(p_device, RG_RES_LTC1, LTC1);
}

GpuTextureId CreateLTC2(IRenderDevice& p_device) {
    return CreateLTC(p_device, RG_RES_LTC2, LTC2);
}

}  // namespace cave::render
