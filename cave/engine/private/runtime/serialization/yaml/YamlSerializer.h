#pragma once
#include <yaml-cpp/yaml.h>

#include "engine/private/runtime/serialization/Serializer.h"

#include "engine/private/core/io/file_access.h"

namespace cave {

class YamlSerializer;

// @TODO: move general logic from YamlSerializer to ISerializer
auto LoadYaml(std::string_view path, YAML::Node& node) -> Result<void>;
auto SaveYaml(std::string_view path, YamlSerializer& serializer) -> Result<void>;

class YamlSerializer : public ISerializer {
public:
    using ISerializer::write;

    ISerializer& beginArray(bool single_line) override;
    ISerializer& endArray() override;

    ISerializer& beginMap(bool single_line) override;
    ISerializer& endMap() override;

    ISerializer& beginKey(std::string_view key) override;

    ISerializer& write(const bool& value) override;
    ISerializer& write(const float& value) override;
    ISerializer& write(const char* value) override;
    ISerializer& write(const std::string& value) override;

    ISerializer& write(const int8_t& value) override;
    ISerializer& write(const uint8_t& value) override;
    ISerializer& write(const int16_t& value) override;
    ISerializer& write(const uint16_t& value) override;
    ISerializer& write(const int32_t& value) override;
    ISerializer& write(const uint32_t& value) override;
    ISerializer& write(const int64_t& value) override;
    ISerializer& write(const uint64_t& value) override;

    ISerializer& write(const Guid& object) override;

    YAML::Emitter& emitter() {
        return out_;
    }

private:
    YAML::Emitter out_;
};

}  // namespace cave

namespace cave {

// @TODO:
static constexpr char BIN_GUARD_MAGIC[] = "SEETHIS";

static inline Result<void> FileWrite(FileAccess* file, const void* data, size_t length) {
    const size_t written = file->WriteBuffer(data, length);
    if (written != length) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CANT_WRITE, "failed to write {} bytes, only wrote {}", length, written);
    }
    return Result<void>();
}

template<TriviallyCopyable T>
static inline Result<void> FileWrite(FileAccess* file, const T& data) {
    return FileWrite(file, &data, sizeof(T));
}

static inline Result<void> FileRead(FileAccess* file, void* data, size_t length) {
    const size_t read = file->ReadBuffer(data, length);
    if (read != length) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CANT_READ, "failed to read {} bytes, only read {}", length, read);
    }
    return Result<void>();
}

template<TriviallyCopyable T>
static inline Result<void> FileRead(FileAccess* file, T& data) {
    return FileRead(file, &data, sizeof(T));
}

}  // namespace cave
