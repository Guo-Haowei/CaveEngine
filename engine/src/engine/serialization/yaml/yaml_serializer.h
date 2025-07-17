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

    ISerializer& BeginArray(bool p_single_line = false) override;
    ISerializer& EndArray() override;

    ISerializer& BeginMap(bool p_single_line = false) override;
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
    FieldFlag flags;
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

#if 0
template<typename T>
Result<void> SerializaYamlVecBinary(YAML::Emitter& p_out, const std::vector<T>& p_object, YamlSerializer& p_serializer) {
    DEV_ASSERT(p_serializer.flags & FieldFlag::BINARY);
    auto& file = p_serializer.file;
    DEV_ASSERT(file);

    const size_t count = p_object.size();
    const size_t size_in_byte = sizeof(p_object[0]) * count;

    long offset = 0;
    if (size_in_byte > 0) {
        if (auto res = FileWrite(file, BIN_GUARD_MAGIC); !res) {
            return CAVE_ERROR(res.error());
        }
        if (auto res = FileWrite(file, size_in_byte); !res) {
            return CAVE_ERROR(res.error());
        }
        // @TODO: better file API
        offset = file->Tell();
        DEV_ASSERT(offset > 0);
        if (auto res = FileWrite(file, p_object.data(), size_in_byte); !res) {
            return CAVE_ERROR(res.error());
        }
    }

    p_out.SetMapFormat(YAML::Flow);
    p_out << YAML::BeginMap;
    p_out << YAML::Key << "offset" << YAML::Value << offset;
    p_out << YAML::Key << "buffer_size" << YAML::Value << size_in_byte;
    p_out << YAML::EndMap;
    p_out.SetMapFormat(YAML::Block);
    return Result<void>();
}

template<typename T>
Result<void> SerializeYaml(YAML::Emitter& p_out, const std::vector<T>& p_object, YamlSerializer& p_serializer) {
    return (p_serializer.flags & FieldFlag::BINARY) ? SerializaYamlVecBinary(p_out, p_object, p_serializer) : SerializaYamlVec(p_out, p_object, p_serializer);
}

#endif

}  // namespace cave
