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

#define REGISTER_FIELD(TYPE, NAME, FIELD, HINT)                                        \
    ::cave::MetaDataTable<TYPE>::RegisterField(((const TYPE*)0)->FIELD,                \
                                               NAME,                                   \
                                               typeid(((const TYPE*)0)->FIELD).name(), \
                                               offsetof(TYPE, FIELD),                  \
                                               HINT)

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

enum class EditorHint {
    None = 0,
    Asset,
    Toggle,
    Color,
    Position,
    Rotation,
    Scale,
};

class ISerializer;
class IDeserializer;

struct FieldMetaBase {
    const char* const name;
    const char* const type;
    const size_t offset;
    const EditorHint editor_hint;

    FieldMetaBase(const char* p_name,
                  const char* p_type,
                  size_t p_offset,
                  EditorHint p_hint)
        : name(p_name), type(p_type), offset(p_offset), editor_hint(p_hint) {
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

    virtual ISerializer& Write(ISerializer& p_serializer, const void* p_object) const = 0;
    virtual bool Read(IDeserializer& p_deserializer, void* p_object) = 0;
};

template<typename T>
struct FieldMeta : FieldMetaBase {
    using FieldMetaBase::FieldMetaBase;

    ISerializer& Write(ISerializer& p_serializer, const void* p_object) const override;
    bool Read(IDeserializer& p_deserializer, void* p_object) override;
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
                                        EditorHint p_hint) {
        return new FieldMeta<U>(p_name, p_type, p_offset, p_hint);
    }
};

}  // namespace cave

#endif
