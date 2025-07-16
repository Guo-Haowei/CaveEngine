#include "yaml_serializer.h"

#include "engine/assets/guid.h"

namespace cave {

YamlSerializer& YamlSerializer::BeginArray(bool p_one_line) {
    m_out.SetSeqFormat(p_one_line ? YAML::Flow : YAML::Block);
    m_out << YAML::BeginSeq;
    return *this;
}

YamlSerializer& YamlSerializer::BeginArray() {
    m_out << YAML::EndSeq;
    return *this;
}

YamlSerializer& YamlSerializer::BeginObject(bool p_one_line) {
    m_out.SetSeqFormat(p_one_line ? YAML::Flow : YAML::Block);
    m_out << YAML::BeginMap;
    return *this;
}

YamlSerializer& YamlSerializer::EndObject() {
    m_out << YAML::EndMap;
    return *this;
}

Result<void> SerializeYaml(YAML::Emitter& p_out, const ecs::Entity& p_object, YamlSerializer&) {
    p_out << p_object.GetId();
    return Result<void>();
}

Result<void> DeserializeYaml(const YAML::Node& p_node, ecs::Entity& p_object, YamlSerializer&) {
    if (!p_node) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Not defined");
    }

    if (!p_node.IsScalar()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Expect scalar");
    }

    p_object = ecs::Entity(p_node.as<uint32_t>());
    return Result<void>();
}

Result<void> SerializeYaml(YAML::Emitter& p_out, const Degree& p_object, YamlSerializer&) {
    p_out << p_object.GetDegree();
    return Result<void>();
}

Result<void> DeserializeYaml(const YAML::Node& p_node, Degree& p_object, YamlSerializer&) {
    if (!p_node) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Not defined");
    }

    if (!p_node.IsScalar()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Expect scalar");
    }

    p_object = Degree(p_node.as<float>());
    return Result<void>();
}

Result<void> SerializeYaml(YAML::Emitter& p_out, const std::string& p_object, YamlSerializer&) {
    p_out << p_object;
    return Result<void>();
}

Result<void> DeserializeYaml(const YAML::Node& p_node, std::string& p_object, YamlSerializer&) {
    if (!p_node) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Not defined");
    }

    if (!p_node.IsScalar()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Expect scalar");
    }

    p_object = p_node.as<std::string>();
    return Result<void>();
}

Result<void> SerializeYaml(YAML::Emitter& p_out, const Guid& p_object, YamlSerializer&) {
    p_out << p_object.ToString();
    return Result<void>();
}

Result<void> DeserializeYaml(const YAML::Node& p_node, Guid& p_object, YamlSerializer&) {
    if (!p_node) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Not defined");
    }

    auto res = Guid::Parse(p_node.as<std::string>());
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    p_object = *res;
    return Result<void>();
}

auto SaveYaml(std::string_view p_path, YAML::Emitter& p_out) -> Result<void> {
    if (!p_out.good()) {
        return CAVE_ERROR(ErrorCode::ERR_PARSE_ERROR, "error: {}", p_out.GetLastError());
    }

    auto res = FileAccess::Open(p_path, FileAccess::WRITE);
    if (!res) {
        return CAVE_ERROR(res.error());
    }
    auto file = *res;

    const char* string = p_out.c_str();
    const size_t len = strlen(string);
    const size_t written = file->WriteBuffer(string, len);
    DEV_ASSERT(written == len);

    return Result<void>();
}

auto LoadYaml(std::string_view p_path, YAML::Node& p_node) -> Result<void> {
    auto res = FileAccess::Open(p_path, FileAccess::READ);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    auto file = *res;

    const size_t size = file->GetLength();
    std::vector<char> buffer;
    buffer.resize(size);
    file->ReadBuffer(buffer.data(), size);
    buffer.push_back('\0');

    p_node = YAML::Load(buffer.data());
    return Result<void>();
}

}  // namespace cave
