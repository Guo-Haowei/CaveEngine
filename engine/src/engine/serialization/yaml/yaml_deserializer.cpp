#include "yaml_deserializer.h"

#include "engine/assets/guid.h"
#include "engine/core/io/file_access.h"

namespace cave {

bool YamlDeserializer::Initialize(const YAML::Node& p_node) {
    const auto& version_node = p_node["version"];

    if (version_node && version_node.IsScalar()) {
        m_version = version_node.as<int>();
    }

    m_stack.emplace_back(p_node);
    m_initialized = true;
    return true;
}

bool YamlDeserializer::TryEnterKey(const char* p_key) {
    auto node = Current()[p_key];
    ERR_FAIL_COND_V_MSG(!node, false, "key not found");

    m_stack.push_back(node);
    return true;
}

void YamlDeserializer::LeaveKey() {
    DEV_ASSERT(!m_stack.empty());

    m_stack.pop_back();
}

bool YamlDeserializer::Read(ecs::Entity& p_object) {
    auto& node = Current();

    ERR_FAIL_COND_V_MSG(!node.IsScalar(), false, "expect scalar");

    p_object = ecs::Entity(node.as<uint32_t>());
    return true;
}

bool YamlDeserializer::Read(Degree& p_object) {
    auto& p_node = Current();

    ERR_FAIL_COND_V_MSG(!p_node.IsScalar(), false, "expect scalar");

    p_object = Degree(p_node.as<float>());
    return true;
}

bool YamlDeserializer::Read(std::string& p_object) {
    auto& p_node = Current();

    ERR_FAIL_COND_V_MSG(!p_node.IsScalar(), false, "expect scalar");

    p_object = p_node.as<std::string>();
    return true;
}

bool YamlDeserializer::Read(Guid& p_object) {
    auto& p_node = Current();

    ERR_FAIL_COND_V_MSG(!p_node.IsScalar(), false, "expect scalar");

    auto res = Guid::Parse(p_node.as<std::string>());
    if (!res) {
        return false;
    }

    p_object = *res;
    return true;
}

bool YamlDeserializer::Read(Matrix4x4f& p_object) {
    auto& p_node = Current();

    ERR_FAIL_COND_V_MSG(!p_node.IsSequence() || p_node.size() != 16, false, "expect matrix4x4");

    float* ptr = &p_object[0].x;
    for (int i = 0; i < 16; ++i) {
        ptr[i] = p_node[i].as<float>();
    }

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
