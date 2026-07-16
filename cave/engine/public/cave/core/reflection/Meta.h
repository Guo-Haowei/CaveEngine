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

#define REGISTER_FIELD(TYPE, NAME, ID, FIELD, ...)                                     \
    ::cave::MetaDataTable<TYPE>::RegisterField(((const TYPE*)0)->FIELD,                \
                                               NAME,                                   \
                                               ID,                                     \
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
    InputText,
    InputInt,
    InputFloat,
    DragFloat,
    DragInt,
    Color,
    Translation,
    Translation2D,
    Rotation,
    Scale,
    BitMask,
    Variant,
    VariantMap,
};

enum class FieldFlag : uint32_t {
    None = 0,
    Serialize = 1,
};

DEFINE_ENUM_BITWISE_OPERATIONS(FieldFlag);

class ISerializer;
class IDeserializer;

struct FieldMetaBase;

struct FieldChange {
    void* object;
    const FieldMetaBase* field;
    const void* old_value;
    const void* new_value;
};

using FieldChangedFn = void (*)(const FieldChange&);

template<typename OwnerT,
         void (OwnerT::*Method)(const FieldChange&)>
void InvokeFieldChanged(const FieldChange& change) {
    auto* object = static_cast<OwnerT*>(change.object);
    (object->*Method)(change);
}

struct FieldMetaBase {
    const char* const name;
    const PropertyId id;
    const size_t offset;
    const FieldFlag flags;
    const EditorHint editor_hint;
    const float v_min;
    const float v_max;

    const FieldChangedFn post_change;

    constexpr FieldMetaBase(const char* name_,
                            PropertyId id_,
                            size_t offset_,
                            FieldFlag flags_,
                            EditorHint hint_,
                            float min_,
                            float max_,
                            FieldChangedFn post_change_ = nullptr) noexcept
        : name(name_)
        , id(id_)
        , offset(offset_)
        , flags(flags_)
        , editor_hint(hint_)
        , v_min(min_)
        , v_max(max_)
        , post_change(post_change_) {
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
                                        const char* name,
                                        PropertyId id,
                                        size_t offset,
                                        FieldFlag flag,
                                        EditorHint hint,
                                        float min = INT_MIN,
                                        float max = INT_MAX) {
        return new FieldMeta<U>(name, id, offset, flag, hint, min, max);
    }
};

}  // namespace cave

#endif
