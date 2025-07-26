#include "pixel_format.h"

namespace cave {

uint32_t ChannelSize(PixelFormat p_format) {
    switch (p_format) {
        case PixelFormat::R8_UINT:
        case PixelFormat::R8G8_UINT:
        case PixelFormat::R8G8B8_UINT:
        case PixelFormat::R8G8B8A8_UINT:
        case PixelFormat::R8G8B8A8_UNORM:
        case PixelFormat::R8G8B8A8_UNORM_SRGB:
            return sizeof(uint8_t);
        case PixelFormat::R16_FLOAT:
        case PixelFormat::R16G16_FLOAT:
        case PixelFormat::R16G16B16_FLOAT:
        case PixelFormat::R16G16B16A16_FLOAT:
            return sizeof(uint16_t);
        case PixelFormat::R32_FLOAT:
        case PixelFormat::R32G32_FLOAT:
        case PixelFormat::R32G32B32_FLOAT:
        case PixelFormat::R32G32B32A32_FLOAT:
        case PixelFormat::D32_FLOAT:
            return sizeof(float);
        default:
            CRASH_NOW();
            return 0;
    }
}

uint32_t ChannelCount(PixelFormat p_format) {
    switch (p_format) {
        case PixelFormat::R8_UINT:
        case PixelFormat::R16_FLOAT:
        case PixelFormat::R32_FLOAT:
        case PixelFormat::D32_FLOAT:
            return 1;
        case PixelFormat::R8G8_UINT:
        case PixelFormat::R16G16_FLOAT:
        case PixelFormat::R32G32_FLOAT:
            return 2;
        case PixelFormat::R8G8B8_UINT:
        case PixelFormat::R16G16B16_FLOAT:
        case PixelFormat::R32G32B32_FLOAT:
            return 3;
        case PixelFormat::R8G8B8A8_UINT:
        case PixelFormat::R16G16B16A16_FLOAT:
        case PixelFormat::R32G32B32A32_FLOAT:
        case PixelFormat::R8G8B8A8_UNORM:
        case PixelFormat::R8G8B8A8_UNORM_SRGB:
            return 4;
        default:
            CRASH_NOW();
            return 0;
    }
}

}  // namespace cave
