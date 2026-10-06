#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/stb_image_write.h>

#include "cave/core/Color.h"
#include "cave/core/math/Vec.h"
#include "cave/core/threading/JobSystem.h"
#include "cave/core/threading/Threads.h"
#include "cave/core/time/Stopwatch.h"

#include "engine/private/core/math/geomath.h"
#include "engine/private/runtime/framework/Engine.h"

// @TODO: refactor
#include "pbr.hlsl.h"
#define EMPTY_APPLICATION
#include "engine/private/runtime/framework/EntryPoint.h"

using namespace cave;

static void WriteImageWrapper(const char* image_path, std::function<void(const char*)> func) {
    Stopwatch stopwatch;
    stopwatch.restart();
    func(image_path);
    stopwatch.stop();
    LOG_OK("Result written to '{}' in {}", image_path, stopwatch.elapsed().ToString());
}

void WriteBrdfImage(const char* image_path) {
    constexpr int width = 512;
    constexpr int height = 512;
    constexpr int job_count = width * height;
    constexpr int channels = 3;

    float* image_data = new float[width * height * channels];
    jobsystem::Context ctx;
    ctx.Dispatch(job_count, 256, [&](jobsystem::JobArgs args) {
        const int index = args.jobIndex;
        const int x = index % width;
        const int y = index / width;
        const float u = (x + 0.5f) / (float)(width);
        const float v = 1.0f - (y + 0.5f) / (float)(height);
        math::Vec2f color = IntegrateBRDF(u, v);
        image_data[channels * index + 0] = color.r;
        image_data[channels * index + 1] = color.g;
        image_data[channels * index + 2] = 0.0f;
    });
    ctx.Wait();

    stbi_write_hdr(image_path, width, height, channels, image_data);

    delete[] image_data;
}

void WriteCheckerBoardImage(const char* image_path) {
    constexpr int channels = 4;

    constexpr int grid_size = 8 * 4;
    constexpr int tex_size = 64 * 4;

    struct Pixel {
        uint8_t r, g, b, a;
    };

    constexpr Pixel light{ 204, 204, 204, 255 };
    constexpr Pixel dark{ 136, 136, 136, 255 };

    std::vector<Pixel> pixels;
    pixels.reserve(tex_size * tex_size);
    for (int y = 0; y < tex_size; ++y) {
        for (int x = 0; x < tex_size; ++x) {
            bool light_tile = ((x / grid_size) + (y / grid_size)) % 2 == 0;
            Pixel pixel = light_tile ? light : dark;
            pixels.push_back(pixel);
        }
    }

    stbi_write_png(image_path, tex_size, tex_size, channels, pixels.data(), tex_size * sizeof(Pixel));
}

void WriteAviatorSkyImage(const char* image_path) {
    constexpr int width = 2048;
    constexpr int height = 1024;
    constexpr int job_count = width * height;
    constexpr int channels = 4;

    auto bottom = Color::Hex((ColorCode)0XE4E0BA);
    auto top = Color::Hex((ColorCode)0xF7D9AA);

    float* image_data = new float[width * height * channels];
    jobsystem::Context ctx;
    ctx.Dispatch(job_count, 256, [&](jobsystem::JobArgs args) {
        const int index = args.jobIndex;
        const int y = index / width;
        float v = 1.0f - (y + 0.5f) / (float)(height);
        auto color = lerp(top, bottom, v);
        image_data[channels * index + 0] = color.r;
        image_data[channels * index + 1] = color.g;
        image_data[channels * index + 2] = color.b;
        image_data[channels * index + 3] = 1.0f;
    });
    ctx.Wait();

    stbi_write_hdr(image_path, width, height, channels, image_data);

    delete[] image_data;
}

int main(int, const char**) {

    engine::InitializeCore();

    WriteImageWrapper("brdf.hdr", WriteBrdfImage);

    thread::RequestShutdown();
    engine::FinalizeCore();

    return 0;
}
