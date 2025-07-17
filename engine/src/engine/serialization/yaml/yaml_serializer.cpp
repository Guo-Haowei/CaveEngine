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

void YamlSerializer::Serialize(const Guid& p_object) {
    m_out << p_object.ToString();
}

void YamlSerializer::Serialize(const std::string& p_object) {
    m_out << p_object;
}

void YamlSerializer::Serialize(const ecs::Entity& p_object) {
    m_out << p_object.GetId();
}

void YamlSerializer::Serialize(const Degree& p_object) {
    m_out << p_object.GetDegree();
}

void YamlSerializer::Serialize(const Matrix4x4f& p_object) {
    BeginArray(true);
    const float* ptr = &p_object[0].x;
    for (int i = 0; i < 16; ++i) {
        Value(ptr[i]);
    }
    EndArray();
}

void YamlSerializer::Serialize(const TileData& p_tile_data) {
    unused(p_tile_data);
    CRASH_NOW_MSG("TODO:");
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
