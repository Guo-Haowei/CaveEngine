#pragma once
#include "reflection.h"

#if USING(USE_REFLECTION)

namespace YAML {
class Node;
class Emitter;
}  // namespace YAML

namespace cave {

// @TODO: remove macros
#define BEGIN_REGISTRY(TYPE) ::cave::MetaDataTable<TYPE>::BeginRegistry()
#define END_REGISTRY(TYPE)   ::cave::MetaDataTable<TYPE>::EndRegistry()

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif

#define REGISTER_FIELD(TYPE, NAME, FIELD)                                              \
    ::cave::MetaDataTable<TYPE>::RegisterField(((const TYPE*)0)->FIELD,                \
                                               NAME,                                   \
                                               typeid(((const TYPE*)0)->FIELD).name(), \
                                               offsetof(TYPE, FIELD))

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

enum FieldFlag : uint32_t;
class YamlSerializer;

struct FieldMetaBase {
    const char* const name;
    const char* const type;
    const size_t offset;
    const FieldFlag flags;

    FieldMetaBase(const char* p_name,
                  const char* p_type,
                  size_t p_offset,
                  FieldFlag p_flags)
        : name(p_name), type(p_type), offset(p_offset), flags(p_flags) {
    }

    virtual ~FieldMetaBase() = default;

    template<typename T>
    const T& GetData(const void* p_object) const {
        char* ptr = (char*)p_object + offset;
        return *reinterpret_cast<T*>(ptr);
    }

    template<typename T>
    T& GetData(void* p_object) {
        char* ptr = (char*)p_object + offset;
        return *reinterpret_cast<T*>(ptr);
    }

    [[nodiscard]] virtual Result<void> DumpYaml(YAML::Emitter& p_out, const void* p_object, YamlSerializer& p_context) const = 0;
    [[nodiscard]] virtual Result<void> UndumpYaml(const YAML::Node& p_node, void* p_object, YamlSerializer& p_context) = 0;
};

template<typename T>
class FieldMeta : public FieldMetaBase {
    using FieldMetaBase::FieldMetaBase;

    Result<void> DumpYaml(YAML::Emitter& p_out, const void* p_object, YamlSerializer& p_context) const override;

    Result<void> UndumpYaml(const YAML::Node& p_node, void* p_object, YamlSerializer& p_context) override;
};

template<typename T>
class MetaDataTable {
public:
    static const MetaTableFields& GetFields();

private:
    template<typename U>
    static FieldMetaBase* RegisterField(const U&,
                                        const char* p_name,
                                        const char* p_type,
                                        size_t p_offset,
                                        FieldFlag p_flag = FieldFlag::NONE) {
        return new FieldMeta<U>(p_name, p_type, p_offset, p_flag);
    }
};

}  // namespace cave

#endif
