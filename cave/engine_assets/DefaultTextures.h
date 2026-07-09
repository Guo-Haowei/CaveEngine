#pragma once
#include "engine/private/runtime/assets/ImageAsset.h"

namespace cave {

static Ref<ImageAsset> CreateWhite1x1Image() {
    auto image = std::make_shared<ImageAsset>();
    image->format = PixelFormat::R8G8B8A8_UNORM;
    image->width = image->height = 1;
    image->num_channels = 4;
    image->buffer = { 255, 255, 255, 255 };
    return image;
}

static Ref<ImageAsset> CreateCheckerBoardImage() {
    constexpr int kNumChannels = 4;

    constexpr int kGridSize = 8 * 4;
    constexpr int kTexSize = 64 * 4;

    struct Pixel {
        uint8_t r, g, b, a;
    };

    constexpr Pixel light{ 204, 204, 204, 255 };
    constexpr Pixel dark{ 136, 136, 136, 255 };

    std::vector<uint8_t> pixels;
    pixels.reserve(kTexSize * kTexSize);
    for (int y = 0; y < kTexSize; ++y) {
        for (int x = 0; x < kTexSize; ++x) {
            bool light_tile = ((x / kGridSize) + (y / kGridSize)) % 2 == 0;
            Pixel pixel = light_tile ? light : dark;
            pixels.push_back(pixel.r);
            pixels.push_back(pixel.g);
            pixels.push_back(pixel.b);
            pixels.push_back(pixel.a);
        }
    }

    auto image = std::make_shared<ImageAsset>();
    image->format = PixelFormat::R8G8B8A8_UNORM;
    image->width = image->height = kTexSize;
    image->num_channels = kNumChannels;
    image->buffer = std::move(pixels);
    return image;
}

}  // namespace cave
