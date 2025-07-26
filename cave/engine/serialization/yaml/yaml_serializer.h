#pragma once
#include <yaml-cpp/yaml.h>

#include "engine/serialization/serializer.h"

#include "engine/core/io/file_access.h"

namespace cave {

// @TODO: move general logic from YamlSerializer to ISerializer
auto LoadYaml(std::string_view p_path, YAML::Node& p_node) -> Result<void>;

class YamlSerializer;

// @TODO: refactor
auto SaveYaml(std::string_view p_path, YamlSerializer& p_serializer) -> Result<void>;

class YamlSerializer : public ISerializer {
public:
    using ISerializer::Write;

    ISerializer& BeginArray(bool p_single_line) override;
    ISerializer& EndArray() override;

    ISerializer& BeginMap(bool p_single_line) override;
    ISerializer& EndMap() override;

    ISerializer& Key(std::string_view p_key) override;

    ISerializer& Write(const bool& p_value) override;
    ISerializer& Write(const float& p_value) override;
    ISerializer& Write(const char* p_value) override;
    ISerializer& Write(const std::string& p_value) override;

    ISerializer& Write(const int8_t& p_value) override;
    ISerializer& Write(const uint8_t& p_value) override;
    ISerializer& Write(const int16_t& p_value) override;
    ISerializer& Write(const uint16_t& p_value) override;
    ISerializer& Write(const int32_t& p_value) override;
    ISerializer& Write(const uint32_t& p_value) override;
    ISerializer& Write(const int64_t& p_value) override;
    ISerializer& Write(const uint64_t& p_value) override;

    ISerializer& Write(const Guid& p_object) override;

    YAML::Emitter& GetEmitter() {
        return m_out;
    }

public:
    uint32_t version;
    FileAccess* file;

    YAML::Emitter m_out;
};

}  // namespace cave

namespace cave {

// @TODO:
static constexpr char BIN_GUARD_MAGIC[] = "SEETHIS";

static inline Result<void> FileWrite(FileAccess* p_file, const void* p_data, size_t p_length) {
    const size_t written = p_file->WriteBuffer(p_data, p_length);
    if (written != p_length) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CANT_WRITE, "failed to write {} bytes, only wrote {}", p_length, written);
    }
    return Result<void>();
}

template<TriviallyCopyable T>
static inline Result<void> FileWrite(FileAccess* p_file, const T& p_data) {
    return FileWrite(p_file, &p_data, sizeof(T));
}

static inline Result<void> FileRead(FileAccess* p_file, void* p_data, size_t p_length) {
    const size_t read = p_file->ReadBuffer(p_data, p_length);
    if (read != p_length) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CANT_READ, "failed to read {} bytes, only read {}", p_length, read);
    }
    return Result<void>();
}

template<TriviallyCopyable T>
static inline Result<void> FileRead(FileAccess* p_file, T& p_data) {
    return FileRead(p_file, &p_data, sizeof(T));
}

}  // namespace cave
