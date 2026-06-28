// =============================================================================
// File: cave/core/reflection/Meta.h
// =============================================================================
#pragma once
#include "cave/core/reflection/Reflection.h"

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

#define REGISTER_FIELD(TYPE, NAME, FIELD, ...)                                         \
    ::cave::MetaDataTable<TYPE>::RegisterField(((const TYPE*)0)->FIELD,                \
                                               NAME,                                   \
                                               typeid(((const TYPE*)0)->FIELD).name(), \
                                               offsetof(TYPE, FIELD),                  \
                                               __VA_ARGS__)

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

enum class EditorHint {
    None = 0,
    EnumDropDown,
    Asset,
    Toggle,
    InputInt,
    InputFloat,
    DragFloat,
    DragInt,
    Color,
    Translation,
    Rotation,
    Scale,
    BitMask,
};

enum class FieldFlag : uint32_t {
    None = 0,
    Serialize = 1,
};

DEFINE_ENUM_BITWISE_OPERATIONS(FieldFlag);

class ISerializer;
class IDeserializer;

struct FieldMetaBase {
    const char* const name;
    const char* const type;
    const PropertyId id;
    const size_t offset;
    const FieldFlag flags;
    const EditorHint editor_hint;
    const float v_min;
    const float v_max;

    constexpr FieldMetaBase(const char* p_name,
                            const char* p_type,
                            size_t p_offset,
                            FieldFlag p_flags,
                            EditorHint p_hint,
                            float p_min,
                            float p_max) noexcept
        : name(p_name)
        , type(p_type)
        , id(PropertyId(p_name))
        , offset(p_offset)
        , flags(p_flags)
        , editor_hint(p_hint)
        , v_min(p_min)
        , v_max(p_max) {
    }

    virtual ~FieldMetaBase() = default;

    template<typename T>
    T& GetData(const void* p_object) const {
        char* ptr = (char*)p_object + offset;
        return *reinterpret_cast<T*>(ptr);
    }

    virtual ISerializer& Write(ISerializer& p_serializer, const void* p_object) const = 0;
    virtual bool Read(IDeserializer& p_deserializer, void* p_object) const = 0;

#if USING(USE_EDITOR)
    virtual bool DrawEditor(void*, float) const = 0;
#endif
};

template<typename T>
struct FieldMeta : FieldMetaBase {
    using FieldMetaBase::FieldMetaBase;

    ISerializer& Write(ISerializer& p_serializer, const void* p_object) const override;
    bool Read(IDeserializer& p_deserializer, void* p_object) const override;

#if USING(USE_EDITOR)
    bool DrawEditor(void* p_object, float p_column_width) const override;
#endif
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
                                        FieldFlag p_flag,
                                        EditorHint p_hint,
                                        float p_min = INT_MIN,
                                        float p_max = INT_MAX) {
        return new FieldMeta<U>(p_name, p_type, p_offset, p_flag, p_hint, p_min, p_max);
    }
};

}  // namespace cave

#endif
