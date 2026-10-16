#pragma once
#include "cave/render/rhi/PixelFormat.h"

namespace cave {

constexpr int kIBLMipChainMax = 7;

constexpr PixelFormat RT_FMT_GBUFFER_DEPTH = PixelFormat::R32G8X24_TYPELESS;
constexpr PixelFormat RT_FMT_GBUFFER_BASE_COLOR = PixelFormat::R16G16B16A16_FLOAT;
constexpr PixelFormat RT_FMT_GBUFFER_NORMAL = PixelFormat::R16G16B16A16_FLOAT;
constexpr PixelFormat RT_FMT_GBUFFER_MATERIAL = PixelFormat::R16G16B16A16_FLOAT;
// @TODO: debug
constexpr PixelFormat kFormatSsao = PixelFormat::R32_FLOAT;
constexpr PixelFormat kFormatTone = PixelFormat::R16G16B16A16_FLOAT;
constexpr PixelFormat kFormatLighting = PixelFormat::R16G16B16A16_FLOAT;
constexpr PixelFormat kFormatOutlineSelect = PixelFormat::R8_UNORM;
// @TODO: rename
constexpr PixelFormat kDefaultSurfaceFormat = PixelFormat::R8G8B8A8_UNORM;
constexpr PixelFormat kDefaultDepthStencilFormat = PixelFormat::D32_FLOAT;

}  // namespace cave
