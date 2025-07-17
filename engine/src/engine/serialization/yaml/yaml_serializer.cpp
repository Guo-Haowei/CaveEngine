#include "yaml_serializer.h"

#include "engine/assets/guid.h"

namespace cave {

ISerializer& YamlSerializer::BeginArray(bool p_single_line) {
    m_out.SetSeqFormat(p_single_line ? YAML::Flow : YAML::Block);
    m_out << YAML::BeginSeq;
    return *this;
}

ISerializer& YamlSerializer::EndArray() {
    m_out << YAML::EndSeq;
    return *this;
}

ISerializer& YamlSerializer::BeginMap(bool p_single_line) {
    m_out.SetSeqFormat(p_single_line ? YAML::Flow : YAML::Block);
    m_out << YAML::BeginMap;
    return *this;
}

ISerializer& YamlSerializer::EndMap() {
    m_out << YAML::EndMap;
    return *this;
}

ISerializer& YamlSerializer::Key(std::string_view p_key) {
    m_out << YAML::Key << p_key << YAML::Value;
    return *this;
}

ISerializer& YamlSerializer::Write(const int8_t& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const uint8_t& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const int16_t& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const uint16_t& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const int32_t& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const uint32_t& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const int64_t& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const uint64_t& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const bool& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const float& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const char* p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const std::string& p_value) {
    m_out << p_value;
    return *this;
}

ISerializer& YamlSerializer::Write(const Guid& p_object) {
    return Write(p_object.ToString());
}

auto SaveYaml(std::string_view p_path, YamlSerializer& p_serializer) -> Result<void> {
    auto& emitter = p_serializer.GetEmitter();

    if (!emitter.good()) {
        return CAVE_ERROR(ErrorCode::ERR_PARSE_ERROR, "error: {}", emitter.GetLastError());
    }

    auto res = FileAccess::Open(p_path, FileAccess::WRITE);
    if (!res) {
        return CAVE_ERROR(res.error());
    }
    auto file = *res;

    const char* string = emitter.c_str();
    const size_t len = strlen(string);
    const size_t written = file->WriteBuffer(string, len);
    DEV_ASSERT(written == len);

    return Result<void>();
}

}  // namespace cave
