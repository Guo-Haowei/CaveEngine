#include "YamlDeserializer.h"

#include "cave/core/ids/Guid.h"
#include "engine/private/core/io/file_access.h"

namespace cave {

YamlDeserializer::~YamlDeserializer() {
    DEV_ASSERT(m_node_stack.size() == 1);  // only root node is left
}

bool YamlDeserializer::initialize(const YAML::Node& node) {
    const auto& version_node = node["version"];

    if (version_node && version_node.IsScalar()) {
        m_version = version_node.as<int>();
    }

    m_node_stack.emplace_back(node);
    m_initialized = true;
    return true;
}

bool YamlDeserializer::tryEnterKey(const char* key) {
    auto node = current()[key];
    if (!node) {
        return false;
    }

#if USING(VALIDATE_SERIALIZER)
    m_type_stack.push_back(SerializerState::Map);
#endif

    m_node_stack.push_back(node);
    return true;
}

void YamlDeserializer::leaveKey() {
    DEV_ASSERT(!m_node_stack.empty());

#if USING(VALIDATE_SERIALIZER)
    DEV_ASSERT(m_type_stack.back() == SerializerState::Map);
    m_type_stack.pop_back();
#endif

    m_node_stack.pop_back();
}

bool YamlDeserializer::tryEnterIndex(int index) {
    const YAML::Node& node = current();

    if (!node || !node.IsSequence()) {
        return false;
    }

    if (index < 0 || static_cast<std::size_t>(index) >= node.size()) {
        return false;
    }

#if USING(VALIDATE_SERIALIZER)
    m_type_stack.push_back(SerializerState::Array);
#endif

    m_node_stack.push_back(node[static_cast<std::size_t>(index)]);
    return true;
}

void YamlDeserializer::leaveIndex() {
    DEV_ASSERT(!m_node_stack.empty());

#if USING(VALIDATE_SERIALIZER)
    DEV_ASSERT(m_type_stack.back() == SerializerState::Array);
    m_type_stack.pop_back();
#endif

    m_node_stack.pop_back();
}

Option<int> YamlDeserializer::arraySize() {
    const auto& top = current();
    if (top && top.IsSequence()) {
        return Some(static_cast<int>(top.size()));
    }
    return None();
}

Option<Vector<String>> YamlDeserializer::getKeys() {
    const auto& top = current();
    if (DEV_VERIFY(top.IsMap())) {
        Vector<String> keys;
        keys.reserve(top.size());
        for (const auto& kv : top) {
            keys.push_back(kv.first.as<std::string>());
        }
        return Some(keys);
    }
    return None();
}

const YAML::Node& YamlDeserializer::current() const {
    DEV_ASSERT(!m_node_stack.empty());
    return m_node_stack.back();
}

template<typename T>
bool YamlDeserializer::readScalar(T& p_out) {
    auto& node = current();
    ERR_FAIL_COND_V_MSG(!node.IsScalar(), false, "expect scalar");
    p_out = node.as<T>();
    return true;
}

bool YamlDeserializer::read(bool& value) {
    return readScalar(value);
}

bool YamlDeserializer::read(float& value) {
    return readScalar(value);
}

bool YamlDeserializer::read(String& value) {
    return readScalar(value);
}

bool YamlDeserializer::read(int8_t& value) {
    return readScalar(value);
}

bool YamlDeserializer::read(uint8_t& value) {
    return readScalar(value);
}

bool YamlDeserializer::read(int16_t& value) {
    return readScalar(value);
}

bool YamlDeserializer::read(uint16_t& value) {
    return readScalar(value);
}

bool YamlDeserializer::read(int32_t& value) {
    return readScalar(value);
}

bool YamlDeserializer::read(uint32_t& value) {
    return readScalar(value);
}

bool YamlDeserializer::read(int64_t& value) {
    return readScalar(value);
}

bool YamlDeserializer::read(uint64_t& value) {
    return readScalar(value);
}

auto LoadYaml(std::string_view path, YAML::Node& node) -> Result<void> {
    auto res = FileAccess::Open(path, FileAccess::READ);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    auto file = *res;

    const size_t size = file->GetLength();
    Vector<char> buffer;
    buffer.resize(size);
    file->ReadBuffer(buffer.data(), size);
    buffer.push_back('\0');

    node = YAML::Load(buffer.data());
    return Result<void>();
}

}  // namespace cave
