#pragma once
#include "engine/assets/image_asset.h"

namespace cave {

static std::shared_ptr<ImageAsset> CreateCheckerBoardImage() {
    constexpr int NUM_CHANNELS = 4;

    constexpr int GRID_SIZE = 8 * 4;
    constexpr int TEX_SIZE = 64 * 4;

    struct Pixel {
        uint8_t r, g, b, a;
    };

    constexpr Pixel light{ 204, 204, 204, 255 };
    constexpr Pixel dark{ 136, 136, 136, 255 };

    std::vector<uint8_t> pixels;
    pixels.reserve(TEX_SIZE * TEX_SIZE);
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool light_tile = ((x / GRID_SIZE) + (y / GRID_SIZE)) % 2 == 0;
            Pixel pixel = light_tile ? light : dark;
            pixels.push_back(pixel.r);
            pixels.push_back(pixel.g);
            pixels.push_back(pixel.b);
            pixels.push_back(pixel.a);
        }
    }

    auto image = std::make_shared<ImageAsset>();
    image->format = PixelFormat::R8G8B8A8_UINT;
    image->width = image->height = TEX_SIZE;
    image->num_channels = NUM_CHANNELS;
    image->buffer = std::move(pixels);
    return image;
}

}  // namespace cave
