#pragma once
#include "cave/runtime/assets/Builtin.h"

#include "engine/private/runtime/assets/ImageAsset.h"

namespace cave {

namespace {

Ref<ImageAsset> CreateWhite1x1Image() {
    auto image = std::make_shared<ImageAsset>();
    image->format = PixelFormat::R8G8B8A8_UNORM;
    image->width = image->height = 1;
    image->num_channels = 4;
    image->buffer = { 255, 255, 255, 255 };
    return image;
}

Ref<ImageAsset> CreateCheckerBoardImage() {
    using Info = CheckerboardInfo;

    struct Pixel {
        uint8_t r, g, b, a;
    };

    constexpr int kChannelCount = 4;
    constexpr Pixel kLight{ 204, 204, 204, 255 };
    constexpr Pixel kDark{ 136, 136, 136, 255 };

    Vector<uint8_t> pixels;
    pixels.reserve(Info::kTextureSizePx * Info::kTextureSizePx * kChannelCount);

    for (int y = 0; y < Info::kTextureSizePx; ++y) {
        for (int x = 0; x < Info::kTextureSizePx; ++x) {
            const bool is_light = ((x / Info::kCellSizePx) + (y / Info::kCellSizePx)) % 2 == 0;
            const Pixel pixel = is_light ? kLight : kDark; 
            pixels.push_back(pixel.r);
            pixels.push_back(pixel.g);
            pixels.push_back(pixel.b);
            pixels.push_back(pixel.a);
        }
    }

    auto image = std::make_shared<ImageAsset>();
    image->format = PixelFormat::R8G8B8A8_UNORM;
    image->width = image->height = Info::kTextureSizePx;
    image->num_channels = kChannelCount;
    image->buffer = std::move(pixels);
    return image;
}

}  // namespace

}  // namespace cave
