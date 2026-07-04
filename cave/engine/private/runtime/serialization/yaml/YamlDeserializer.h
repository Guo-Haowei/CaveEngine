#pragma once
#include <yaml-cpp/yaml.h>

#include "engine/private/runtime/serialization/Deserializer.h"

namespace cave {

class Guid;

auto LoadYaml(std::string_view path, YAML::Node& node) -> Result<void>;

class YamlDeserializer : public IDeserializer {
public:
    using IDeserializer::read;

    // @TODO: make it private
    bool Initialize(const YAML::Node& node);

    ~YamlDeserializer();

    int version() const override {
        DEV_ASSERT(initialized_);
        return version_;
    }

    bool tryEnterKey(const char* key) override;

    void leaveKey() override;

    bool tryEnterIndex(int index) override;

    void leaveIndex() override;

    Option<int> arraySize() override;

    Option<std::vector<std::string>> getKeys() override;

    bool read(bool& value) override;
    bool read(float& value) override;
    bool read(std::string& value) override;

    bool read(int8_t& value) override;
    bool read(uint8_t& value) override;
    bool read(int16_t& value) override;
    bool read(uint16_t& value) override;
    bool read(int32_t& value) override;
    bool read(uint32_t& value) override;
    bool read(int64_t& value) override;
    bool read(uint64_t& value) override;

private:
    const YAML::Node& current() const;

    template<typename T>
    bool readScalar(T& p_out);

#if USING(VALIDATE_SERIALIZER)
    std::vector<SerializerState> type_stack_;
#endif

    std::vector<YAML::Node> node_stack_;
    int version_{ -1 };
    bool initialized_{ false };
};

template<typename T>
bool FieldMeta<T>::Read(IDeserializer& deserializer, void* object) const {
    T& data = FieldMetaBase::GetData<T>(object);

    return deserializer.read(data);
}

}  // namespace cave
