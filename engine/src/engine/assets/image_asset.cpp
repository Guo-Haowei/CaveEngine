#include "image_asset.h"

#include <tinygltf/stb_image.h>

#include "engine/core/io/file_access.h"
#include "engine/core/string/string_utils.h"

namespace cave {

static PixelFormat ChannelToFormat(int p_channel, bool p_is_float) {
    switch (p_channel) {
        case 1:
            return p_is_float ? PixelFormat::R32_FLOAT : PixelFormat::R8_UINT;
        case 2:
            return p_is_float ? PixelFormat::R32G32_FLOAT : PixelFormat::R8G8_UINT;
        case 3:
            return p_is_float ? PixelFormat::R32G32B32_FLOAT : PixelFormat::R8G8B8_UINT;
        case 4:
            return p_is_float ? PixelFormat::R32G32B32A32_FLOAT : PixelFormat::R8G8B8A8_UNORM;
        default:
            CRASH_NOW();
            return PixelFormat::UNKNOWN;
    }
}
static Result<void> LoadImage(const AssetMetaData& p_meta, ImageAsset& p_image) {
    auto res = FileAccess::Open(p_meta.import_path, FileAccess::READ);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    std::string_view extension = StringUtils::Extension(p_meta.import_path);

    // @TODO: improve this part
    const bool is_float = extension == ".hdr";

    // @TODO: get options

    std::shared_ptr<FileAccess> file = *res;
    const size_t size = file->GetLength();
    std::vector<uint8_t> file_buffer;
    file_buffer.resize(size);
    file->ReadBuffer(file_buffer.data(), size);

    int width = 0;
    int height = 0;
    int num_channels = 0;
    // const int req_channel = is_float ? 0 : 4;
    // @TODO: fix this
    const int req_channel = 4;

    uint8_t* pixels = nullptr;
    if (is_float) {
        pixels = (uint8_t*)stbi_loadf_from_memory(file_buffer.data(),
                                                  (uint32_t)size,
                                                  &width,
                                                  &height,
                                                  &num_channels,
                                                  req_channel);
    } else {
        pixels = (uint8_t*)stbi_load_from_memory(file_buffer.data(),
                                                 (uint32_t)size,
                                                 &width,
                                                 &height,
                                                 &num_channels,
                                                 req_channel);
    }

    if (!pixels) {
        return CAVE_ERROR(ErrorCode::ERR_PARSE_ERROR, "failed to parse file '{}'", p_meta.import_path);
    }

    if (req_channel > num_channels) {
        num_channels = req_channel;
    }

    const uint32_t pixel_size = is_float ? sizeof(float) : sizeof(uint8_t);

    int num_pixels = width * height * num_channels;
    std::vector<uint8_t> buffer;
    buffer.resize(pixel_size * num_pixels);
    memcpy(buffer.data(), pixels, pixel_size * num_pixels);
    stbi_image_free(pixels);

    PixelFormat format = ChannelToFormat(num_channels, is_float);

    p_image.format = format;
    p_image.width = width;
    p_image.height = height;
    p_image.num_channels = num_channels;
    p_image.buffer = std::move(buffer);

    return Result<void>();
}

Result<void> ImageAsset::LoadFromDisk(const AssetMetaData& p_meta) {
    sampler = EnumTraits<Sampler>::FromString(p_meta.import_settings["sampler"]).unwrap_or(Sampler::Linear);
    color_space = EnumTraits<ColorSpace>::FromString(p_meta.import_settings["color_space"]).unwrap_or(ColorSpace::Linear);

    return LoadImage(p_meta, *this);
}

Result<void> ImageAsset::SaveToDisk(const AssetMetaData& p_meta) const {
    // @TODO:
    p_meta.import_settings["sampler"] = EnumTraits<Sampler>::ToString(sampler);
    p_meta.import_settings["color_space"] = EnumTraits<ColorSpace>::ToString(color_space);

    return p_meta.SaveToDisk(this);
}

std::vector<Guid> ImageAsset::GetDependencies() const {
    return {};
}

}  // namespace cave
