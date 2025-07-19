#pragma once
#include <yaml-cpp/yaml.h>

#include "engine/serialization/deserializer.h"

namespace cave {

class Guid;
struct TileData;

auto LoadYaml(std::string_view p_path, YAML::Node& p_node) -> Result<void>;

class YamlDeserializer : public IDeserializer {
public:
    using IDeserializer::Read;

    // @TODO: make it private
    bool Initialize(const YAML::Node& p_node);

    int GetVersion() const override {
        DEV_ASSERT(m_initialized);
        return m_version;
    }

    bool TryEnterKey(const char* p_key) override;

    void LeaveKey() override;

    bool TryEnterIndex(int p_index) override;

    void LeaveIndex() override;

    Option<int> ArraySize() override;

    Option<std::vector<std::string>> GetKeys() override;

    bool Read(bool& p_value) override;
    bool Read(float& p_value) override;
    bool Read(std::string& p_value) override;

    bool Read(int8_t& p_value) override;
    bool Read(uint8_t& p_value) override;
    bool Read(int16_t& p_value) override;
    bool Read(uint16_t& p_value) override;
    bool Read(int32_t& p_value) override;
    bool Read(uint32_t& p_value) override;
    bool Read(int64_t& p_value) override;
    bool Read(uint64_t& p_value) override;

    const YAML::Node& Current() {
        DEV_ASSERT(!m_node_stack.empty());
        return m_node_stack.back();
    }

private:
    template<typename T>
    bool ReadScalar(T& p_out);

    std::vector<YAML::Node> m_node_stack;
    int m_version{ -1 };
    bool m_initialized{ false };
};

template<typename T>
bool FieldMeta<T>::Read(IDeserializer& p_deserializer, void* p_object) {
    T& data = FieldMetaBase::GetData<T>(p_object);

    return p_deserializer.Read(data);
}

#if 0
template<typename T, int N>
Result<void> DeserializeYaml(const YAML::Node& node, T (&p_object)[N], YamlSerializer& p_serializer) {
    if (!node) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Not defined");
    }

    if (!node.IsSequence() || node.size() != N) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Expect sequence of {}", N);
    }

    for (int i = 0; i < N; ++i) {
        if (auto res = DeserializeYaml(node[i], p_object[i], p_serializer); !res) {
            return CAVE_ERROR(res.error());
        }
    }

    return Result<void>();
}

template<typename T>
Result<void> DeserializeYamlVec(const YAML::Node& node, std::vector<T>& p_object, YamlSerializer& p_serializer) {
    DEV_ASSERT(!(p_serializer.flags & FieldFlag::BINARY));
    if (!node || !node.IsSequence()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "not a valid sequence");
    }

    const size_t count = node.size();
    p_object.resize(count);
    for (size_t i = 0; i < count; ++i) {
        if (auto res = DeserializeYaml(node[i], p_object[i], p_serializer); !res) {
            return CAVE_ERROR(res.error());
        }
    }
    return Result<void>();
}

template<typename T>
Result<void> DeserializeYamlVecBinary(const YAML::Node& node, std::vector<T>& p_object, YamlSerializer& p_serializer) {
    DEV_ASSERT(p_serializer.flags & FieldFlag::BINARY);
    constexpr size_t element_size = sizeof(p_object[0]);
    constexpr size_t internal_offset = sizeof(BIN_GUARD_MAGIC) + sizeof(size_t);
    if (!node || !node.IsMap()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "not a valid buffer, expect (length, offset)");
    }

    auto& file = p_serializer.file;
    DEV_ASSERT(file);

    size_t offset = 0;
    size_t size_in_byte = 0;
    if (auto res = DeserializeYaml(node["offset"], offset, p_serializer); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = DeserializeYaml(node["buffer_size"], size_in_byte, p_serializer); !res) {
        return CAVE_ERROR(res.error());
    }
    if (size_in_byte == 0) {
        return Result<void>();
    }
    DEV_ASSERT(size_in_byte % element_size == 0);
    DEV_ASSERT(offset >= internal_offset);

    const size_t element_count = size_in_byte / element_size;

    if (auto seek = file->Seek((long)(offset - internal_offset)); seek != 0) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CANT_READ, "Seek failed");
    }

    char magic[sizeof(BIN_GUARD_MAGIC)];
    if (auto res = FileRead(file, magic); !res) {
        return CAVE_ERROR(res.error());
    }

    if (!StringUtils::StringEqual(magic, BIN_GUARD_MAGIC)) {
        magic[sizeof(BIN_GUARD_MAGIC) - 1] = 0;
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "wrong magic {}", magic);
    }

    size_t stored_length = 0;
    if (auto res = FileRead(file, stored_length); !res) {
        return CAVE_ERROR(res.error());
    }

    if (stored_length != size_in_byte) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "wrong size (cache: {})", stored_length);
    }

    p_object.resize(element_count);
    if (auto res = FileRead(file, p_object.data(), size_in_byte); !res) {
        return CAVE_ERROR(res.error());
    }

    return Result<void>();
}
#endif

}  // namespace cave
