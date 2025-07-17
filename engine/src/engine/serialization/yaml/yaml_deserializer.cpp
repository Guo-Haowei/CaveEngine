#include "yaml_deserializer.h"

#include "engine/assets/guid.h"
#include "engine/core/io/file_access.h"

namespace cave {

Option<int> YamlDeserializer::GetVersion() const {
    const auto& version_node = m_node["version"];
    if (version_node && version_node.IsScalar()) {
        return version_node.as<int>();
    }

    return Option<int>::None();
}

bool YamlDeserializer::Deserialize(const YAML::Node& p_node, ecs::Entity& p_object) {
    ERR_FAIL_COND_V_MSG(!p_node.IsScalar(), false, "expect scalar");

    p_object = ecs::Entity(p_node.as<uint32_t>());
    return true;
}

bool YamlDeserializer::Deserialize(const YAML::Node& p_node, Degree& p_object) {
    ERR_FAIL_COND_V_MSG(!p_node.IsScalar(), false, "expect scalar");

    p_object = Degree(p_node.as<float>());
    return true;
}

bool YamlDeserializer::Deserialize(const YAML::Node& p_node, std::string& p_object) {
    ERR_FAIL_COND_V_MSG(!p_node.IsScalar(), false, "expect scalar");

    p_object = p_node.as<std::string>();
    return true;
}

bool YamlDeserializer::Deserialize(const YAML::Node& p_node, Guid& p_object) {
    ERR_FAIL_COND_V_MSG(!p_node.IsScalar(), false, "expect scalar");

    auto res = Guid::Parse(p_node.as<std::string>());
    if (!res) {
        return false;
    }

    p_object = *res;
    return true;
}

bool YamlDeserializer::Deserialize(const YAML::Node& p_node, Matrix4x4f& p_object) {
    ERR_FAIL_COND_V_MSG(!p_node.IsSequence() || p_node.size() != 16, false, "expect matrix4x4");

    float* ptr = &p_object[0].x;
    for (int i = 0; i < 16; ++i) {
        ptr[i] = p_node[i].as<float>();
    }

    return true;
}

bool YamlDeserializer::Deserialize(const YAML::Node& p_node, const TileData& p_tile_data) {
    CRASH_NOW();
    unused(p_node);
    unused(p_tile_data);
    return true;
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
