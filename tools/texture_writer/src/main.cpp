#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/stb_image_write.h"

#include "engine/core/os/threads.h"
#include "engine/core/os/timer.h"
#include "engine/systems/job_system/job_system.h"
#include "engine/math/geomath.h"
#include "engine/runtime/engine.h"
#include "engine/math/color.h"
#include "engine/math/vector.h"

// @TODO: refactor
#include "pbr.hlsl.h"

using namespace my;

static void WriteImageWrapper(const char* p_file, std::function<void(const char*)> p_func) {
    Timer timer;
    p_func(p_file);
    LOG_OK("Result written to '{}' in {}", p_file, timer.GetDurationString());
}

void WriteBrdfImage(const char* p_file) {
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
        Vector2f color = IntegrateBRDF(u, v);
        image_data[channels * index + 0] = color.r;
        image_data[channels * index + 1] = color.g;
        image_data[channels * index + 2] = 0.0f;
    });
    ctx.Wait();

    stbi_write_hdr(p_file, width, height, channels, image_data);

    delete[] image_data;
}

void WriteCheckerBoardImage(const char* p_file) {
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

    stbi_write_png(p_file, tex_size, tex_size, channels, pixels.data(), tex_size * sizeof(Pixel));
}

void WriteAviatorSkyImage(const char* p_file) {
    constexpr int width = 2048;
    constexpr int height = 1024;
    constexpr int job_count = width * height;
    constexpr int channels = 4;

    auto bottom = Color::Hex(0XE4E0BA);
    auto top = Color::Hex(0xF7D9AA);

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

    stbi_write_hdr(p_file, width, height, channels, image_data);

    delete[] image_data;
}

int main(int, const char**) {

    engine::InitializeCore();

    WriteImageWrapper("checkerboard.png", WriteCheckerBoardImage);

    thread::RequestShutdown();
    engine::FinalizeCore();

    return 0;
}
