#pragma once

namespace cave {

enum class PixelFormat {
    UNKNOWN,

    R8_UINT,
    R8G8_UINT,
    R8G8B8_UINT,
    R8G8B8A8_UINT,

    R8G8B8A8_UNORM,

    R16_FLOAT,
    R16G16_FLOAT,
    R16G16B16_FLOAT,
    R16G16B16A16_FLOAT,

    R32_FLOAT,
    R32G32_FLOAT,
    R32G32B32_FLOAT,
    R32G32B32A32_FLOAT,

    R32G32_SINT,
    R32G32B32_SINT,
    R32G32B32A32_SINT,

    R11G11B10_FLOAT,
    R10G10B10A2_UINT,

    D32_FLOAT,

    R24G8_TYPELESS,
    R24_UNORM_X8_TYPELESS,
    D24_UNORM_S8_UINT,
    X24_TYPELESS_G8_UINT,

    R32G8X24_TYPELESS,
    D32_FLOAT_S8X24_UINT,

    COUNT,
};

// @TODO: refactor
uint32_t ChannelSize(PixelFormat p_format);

uint32_t ChannelCount(PixelFormat p_format);

}  // namespace cave
