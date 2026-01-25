#include "yaml_deserializer.h"

#include "engine/assets/guid.h"
#include "engine/core/io/file_access.h"

namespace cave {

YamlDeserializer::~YamlDeserializer() {
    DEV_ASSERT(m_node_stack.size() == 1);  // only root node is left
}

bool YamlDeserializer::Initialize(const YAML::Node& p_node) {
    const auto& version_node = p_node["version"];

    if (version_node && version_node.IsScalar()) {
        m_version = version_node.as<int>();
    }

    m_node_stack.emplace_back(p_node);
    m_initialized = true;
    return true;
}

bool YamlDeserializer::TryEnterKey(const char* p_key) {
    auto node = Current()[p_key];
    if (!node) {
        return false;
    }

#if USING(VALIDATE_SERIALIZER)
    m_type_stack.push_back(SerializerState::Map);
#endif

    m_node_stack.push_back(node);
    return true;
}

void YamlDeserializer::LeaveKey() {
    DEV_ASSERT(!m_node_stack.empty());

#if USING(VALIDATE_SERIALIZER)
    DEV_ASSERT(m_type_stack.back() == SerializerState::Map);
    m_type_stack.pop_back();
#endif

    m_node_stack.pop_back();
}

bool YamlDeserializer::TryEnterIndex(int p_index) {
    auto node = Current()[p_index];
    ERR_FAIL_COND_V_MSG(!node, false, "index not found");

#if USING(VALIDATE_SERIALIZER)
    m_type_stack.push_back(SerializerState::Array);
#endif

    m_node_stack.push_back(node);
    return true;
}

void YamlDeserializer::LeaveIndex() {
    DEV_ASSERT(!m_node_stack.empty());

#if USING(VALIDATE_SERIALIZER)
    DEV_ASSERT(m_type_stack.back() == SerializerState::Array);
    m_type_stack.pop_back();
#endif

    m_node_stack.pop_back();
}

Option<int> YamlDeserializer::ArraySize() {
    const auto& top = Current();
    if (top && top.IsSequence()) {
        return Some(static_cast<int>(top.size()));
    }
    return None();
}

Option<std::vector<std::string>> YamlDeserializer::GetKeys() {
    const auto& top = Current();
    if (DEV_VERIFY(top.IsMap())) {
        std::vector<std::string> keys;
        keys.reserve(top.size());
        for (const auto& kv : top) {
            keys.push_back(kv.first.as<std::string>());
        }
        return Some(keys);
    }
    return None();
}

template<typename T>
bool YamlDeserializer::ReadScalar(T& p_out) {
    auto& node = Current();
    ERR_FAIL_COND_V_MSG(!node.IsScalar(), false, "expect scalar");
    p_out = node.as<T>();
    return true;
}

bool YamlDeserializer::Read(bool& p_value) {
    return ReadScalar(p_value);
}

bool YamlDeserializer::Read(float& p_value) {
    return ReadScalar(p_value);
}

bool YamlDeserializer::Read(std::string& p_value) {
    return ReadScalar(p_value);
}

bool YamlDeserializer::Read(int8_t& p_value) {
    return ReadScalar(p_value);
}

bool YamlDeserializer::Read(uint8_t& p_value) {
    return ReadScalar(p_value);
}

bool YamlDeserializer::Read(int16_t& p_value) {
    return ReadScalar(p_value);
}

bool YamlDeserializer::Read(uint16_t& p_value) {
    return ReadScalar(p_value);
}

bool YamlDeserializer::Read(int32_t& p_value) {
    return ReadScalar(p_value);
}

bool YamlDeserializer::Read(uint32_t& p_value) {
    return ReadScalar(p_value);
}

bool YamlDeserializer::Read(int64_t& p_value) {
    return ReadScalar(p_value);
}

bool YamlDeserializer::Read(uint64_t& p_value) {
    return ReadScalar(p_value);
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
