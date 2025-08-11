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

    ~YamlDeserializer();

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

#if USING(VALIDATE_SERIALIZER)
    std::vector<SerializerState> m_type_stack;
#endif

    std::vector<YAML::Node> m_node_stack;
    int m_version{ -1 };
    bool m_initialized{ false };
};

template<typename T>
bool FieldMeta<T>::Read(IDeserializer& p_deserializer, void* p_object) {
    T& data = FieldMetaBase::GetData<T>(p_object);

    return p_deserializer.Read(data);
}

}  // namespace cave
