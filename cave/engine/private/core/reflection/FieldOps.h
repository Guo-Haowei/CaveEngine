#pragma once
#include "cave/core/reflection/Meta.h"

#include "engine/private/runtime/serialization/Deserializer.h"
#include "engine/private/runtime/serialization/Serializer.h"

namespace cave {

template<typename T>
struct FieldOpsFor {
    static ISerializer& write(const FieldMetaBase& field, ISerializer& serializer, const void* object) {
        const T& data = field.getData<T>(object);
        return serializer.beginKey(field.name).write(data);
    }

    static bool read(const FieldMetaBase& field, IDeserializer& deserializer, void* object) {
        T& data = field.getData<T>(object);
        return deserializer.read(data);
    }

#if USING(USE_EDITOR)
    static bool draw(const FieldMetaBase& field, void* object, float column_width) {
        if constexpr (HasEnumTraits<T>) {
            T& value = field.getData<T>(object);
            return DrawEnumDropDown<T>(field.name, value, column_width);
        } else {
            return false;
        }
    }
#endif

    static constexpr FieldOps get() {
        return FieldOps{
            &write,
            &read,
#if USING(USE_EDITOR)
            &draw,
#endif
        };
    }
};

}  // namespace cave
