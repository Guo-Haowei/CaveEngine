#include "YamlDeserializer.h"

#include "cave/core/ids/Guid.h"
#include "engine/private/core/io/file_access.h"

namespace cave {

YamlDeserializer::~YamlDeserializer() {
    DEV_ASSERT(node_stack_.size() == 1);  // only root node is left
}

bool YamlDeserializer::Initialize(const YAML::Node& p_node) {
    const auto& version_node = p_node["version"];

    if (version_node && version_node.IsScalar()) {
        version_ = version_node.as<int>();
    }

    node_stack_.emplace_back(p_node);
    initialized_ = true;
    return true;
}

bool YamlDeserializer::tryEnterKey(const char* p_key) {
    auto node = current()[p_key];
    if (!node) {
        return false;
    }

#if USING(VALIDATE_SERIALIZER)
    type_stack_.push_back(SerializerState::Map);
#endif

    node_stack_.push_back(node);
    return true;
}

void YamlDeserializer::leaveKey() {
    DEV_ASSERT(!node_stack_.empty());

#if USING(VALIDATE_SERIALIZER)
    DEV_ASSERT(type_stack_.back() == SerializerState::Map);
    type_stack_.pop_back();
#endif

    node_stack_.pop_back();
}

bool YamlDeserializer::tryEnterIndex(int p_index) {
    auto node = current()[p_index];
    ERR_FAIL_COND_V_MSG(!node, false, "index not found");

#if USING(VALIDATE_SERIALIZER)
    type_stack_.push_back(SerializerState::Array);
#endif

    node_stack_.push_back(node);
    return true;
}

void YamlDeserializer::leaveIndex() {
    DEV_ASSERT(!node_stack_.empty());

#if USING(VALIDATE_SERIALIZER)
    DEV_ASSERT(type_stack_.back() == SerializerState::Array);
    type_stack_.pop_back();
#endif

    node_stack_.pop_back();
}

Option<int> YamlDeserializer::arraySize() {
    const auto& top = current();
    if (top && top.IsSequence()) {
        return Some(static_cast<int>(top.size()));
    }
    return None();
}

Option<std::vector<std::string>> YamlDeserializer::getKeys() {
    const auto& top = current();
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

const YAML::Node& YamlDeserializer::current() const {
    DEV_ASSERT(!node_stack_.empty());
    return node_stack_.back();
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

bool YamlDeserializer::read(std::string& value) {
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
    std::vector<char> buffer;
    buffer.resize(size);
    file->ReadBuffer(buffer.data(), size);
    buffer.push_back('\0');

    node = YAML::Load(buffer.data());
    return Result<void>();
}

}  // namespace cave
