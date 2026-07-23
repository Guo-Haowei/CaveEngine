// =============================================================================
// File: cave/core/reflection/Meta.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
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

#define REGISTER_FIELD(TYPE, NAME, ID, FIELD, ...)                              \
    ::cave::MetaDataTable<TYPE>::registerField(((const TYPE*)0)->FIELD,         \
                                               NAME,                            \
                                               ID,                              \
                                               offsetof(TYPE, FIELD),           \
                                               sizeof(((const TYPE*)0)->FIELD), \
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
class Scene;

struct FieldMetaBase;

struct FieldChange {
    Scene* scene;
    ecs::Entity entity;
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

struct FieldOps {
    using WriteFn = ISerializer& (*)(const FieldMetaBase&, ISerializer&, const void*);
    using ReadFn = bool (*)(const FieldMetaBase&, IDeserializer&, void*);

#if USING(USE_EDITOR)
    using DrawFn = bool (*)(const FieldMetaBase&, void*, float);
#endif

    WriteFn write = nullptr;
    ReadFn read = nullptr;

#if USING(USE_EDITOR)
    DrawFn draw = nullptr;
#endif
};

struct FieldMetaBase {
    const char* const name;
    const PropertyId id;
    const uint32_t offset;
    const uint32_t size;
    const FieldFlag flags;
    const EditorHint editor_hint;
    const float v_min;
    const float v_max;

    const FieldChangedFn on_change;
    const FieldOps ops;

    constexpr FieldMetaBase(const char* name,
                            PropertyId id,
                            uint32_t offset,
                            uint32_t size,
                            FieldFlag flags,
                            EditorHint hint,
                            FieldChangedFn on_change,
                            FieldOps ops,
                            float min,
                            float max) noexcept
        : name(name)
        , id(id)
        , offset(offset)
        , size(size)
        , flags(flags)
        , editor_hint(hint)
        , v_min(min)
        , v_max(max)
        , on_change(on_change)
        , ops(ops) {
    }

    virtual ~FieldMetaBase() = default;

    template<typename T>
    T& getData(const void* object) const {
        char* ptr = (char*)object + offset;
        return *reinterpret_cast<T*>(ptr);
    }

    const void* getRaw(const void* object) const {
        return reinterpret_cast<const char*>(object) + offset;
    }

    void* getRaw(void* object) const {
        return reinterpret_cast<char*>(object) + offset;
    }

    ISerializer& write(ISerializer& serializer, const void* object) const {
        return ops.write(*this, serializer, object);
    }

    bool read(IDeserializer& deserializer, void* object) const {
        return ops.read(*this, deserializer, object);
    }

#if USING(USE_EDITOR)
    bool drawEditor(void* object, float width) const {
        return ops.draw(*this, object, width);
    }
#endif
};

template<typename T>
class MetaDataTable {
public:
    static const MetaTableFields& getFields();

private:
    template<typename U>
    static FieldMetaBase registerField(const U&,
                                       const char* name,
                                       PropertyId id,
                                       uint32_t offset,
                                       uint32_t size,
                                       FieldFlag flag,
                                       EditorHint hint,
                                       FieldChangedFn on_change,
                                       FieldOps ops,
                                       float min = INT_MIN,
                                       float max = INT_MAX) {
        return FieldMetaBase(name,
                             id,
                             offset,
                             size,
                             flag,
                             hint,
                             on_change,
                             ops,
                             min,
                             max);
    }
};

}  // namespace cave

#endif
