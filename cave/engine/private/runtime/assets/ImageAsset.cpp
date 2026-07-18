#include "ImageAsset.h"

#include <tinygltf/stb_image.h>

#include "engine/private/core/io/file_access.h"
#include "cave/core/string/StringUtils.h"

namespace cave {

static PixelFormat ChannelToFormat(int channel, bool is_float) {
    switch (channel) {
        case 1:
            return is_float ? PixelFormat::R32_FLOAT : PixelFormat::R8_UINT;
        case 2:
            return is_float ? PixelFormat::R32G32_FLOAT : PixelFormat::R8G8_UINT;
        case 3:
            return is_float ? PixelFormat::R32G32B32_FLOAT : PixelFormat::R8G8B8_UINT;
        case 4:
            return is_float ? PixelFormat::R32G32B32A32_FLOAT : PixelFormat::R8G8B8A8_UNORM;
        default:
            CRASH_NOW();
            return PixelFormat::UNKNOWN;
    }
}

static Result<void> LoadImage(const AssetMetaData& meta, ImageAsset& image) {
    auto res = FileAccess::Open(meta.import_path, FileAccess::READ);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    std::string_view extension = StringUtils::extension(meta.import_path);

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
        return CAVE_ERROR(ErrorCode::ERR_PARSE_ERROR, "failed to parse file '{}'", meta.import_path);
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
    if (format == PixelFormat::R8G8B8A8_UNORM && image.color_space == ImageAsset::ColorSpace::SRGB) {
        format = PixelFormat::R8G8B8A8_UNORM_SRGB;
    }

    image.format = format;
    image.width = width;
    image.height = height;
    image.num_channels = num_channels;
    image.buffer = std::move(buffer);

    return Result<void>();
}

Result<void> ImageAsset::loadFromDisk(const AssetMetaData& meta) {
    address_mode = EnumTraits<AddressMode>::FromString(meta.import_settings["address_mode"]).unwrap_or(AddressMode::Clamp);
    sampler = EnumTraits<Sampler>::FromString(meta.import_settings["sampler"]).unwrap_or(Sampler::Linear);
    color_space = EnumTraits<ColorSpace>::FromString(meta.import_settings["color_space"]).unwrap_or(ColorSpace::Linear);

    return LoadImage(meta, *this);
}

Result<void> ImageAsset::saveToDisk(const AssetMetaData& meta) const {
    meta.import_settings["sampler"] = EnumTraits<Sampler>::ToString(sampler);
    meta.import_settings["color_space"] = EnumTraits<ColorSpace>::ToString(color_space);

    return meta.saveToDisk(this);
}

Vector<Guid> ImageAsset::dependencies() const {
    return {};
}

}  // namespace cave
