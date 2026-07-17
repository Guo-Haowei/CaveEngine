#pragma once
#include "engine/private/renderer/pixel_format.h"

namespace cave {

#define RG_PASS_BLOOM_DOWN_PREFIX "p:bloom_downsample"
#define RG_PASS_BLOOM_UP_PREFIX   "p:bloom_upsample"
#define RG_RES_BLOOM_PREFIX       "r:bloom"

constexpr int kBloomMipChainMax = 7;
constexpr int kIBLMipChainMax = 7;

constexpr int RT_SIZE_IBL_CUBEMAP = 512;
constexpr int RT_SIZE_IBL_IRRADIANCE_CUBEMAP = 32;
constexpr int RT_SIZE_IBL_PREFILTERED_CUBEMAP = 512;
// RT_FMT stands form RENDER_TARGET_FORMAT
constexpr PixelFormat RT_FMT_GBUFFER_DEPTH = PixelFormat::R32G8X24_TYPELESS;
// constexpr PixelFormat RT_FMT_GBUFFER_DEPTH = PixelFormat::R24G8_TYPELESS;
constexpr PixelFormat RT_FMT_GBUFFER_BASE_COLOR = PixelFormat::R16G16B16A16_FLOAT;
constexpr PixelFormat RT_FMT_GBUFFER_POSITION = PixelFormat::R16G16B16A16_FLOAT;
constexpr PixelFormat RT_FMT_GBUFFER_NORMAL = PixelFormat::R16G16B16A16_FLOAT;
constexpr PixelFormat RT_FMT_GBUFFER_MATERIAL = PixelFormat::R16G16B16A16_FLOAT;
// @TODO: debug
constexpr PixelFormat RT_FMT_SSAO = PixelFormat::R32_FLOAT;
constexpr PixelFormat RT_FMT_TONE = PixelFormat::R16G16B16A16_FLOAT;
constexpr PixelFormat RT_FMT_LIGHTING = PixelFormat::R16G16B16A16_FLOAT;
constexpr PixelFormat RT_FMT_OUTLINE_SELECT = PixelFormat::R8_UINT;
// @TODO: rename
constexpr PixelFormat DEFAULT_SURFACE_FORMAT = PixelFormat::R8G8B8A8_UNORM;
constexpr PixelFormat DEFAULT_DEPTH_STENCIL_FORMAT = PixelFormat::D32_FLOAT;

}  // namespace cave
