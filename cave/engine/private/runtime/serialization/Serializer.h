#pragma once
// @TODO: refactor defines.h
#include "defines.h"

#include "cave/core/ids/Entity.h"
#include "cave/core/math/Angle.h"
#include "cave/core/math/Box.h"
#include "cave/core/math/Matrix.h"
#include "cave/core/containers/FixedStack.h"
#include "cave/core/containers/FixedString.h"

#include "engine/private/core/io/file_access.h"

namespace cave {

class Guid;
class Variant;

#if USING(VALIDATE_SERIALIZER)
#define IF_VALIDATE_SERIALIZER(x) \
    do { x; } while (0)
#else
#define IF_VALIDATE_SERIALIZER(x) (void)0
#endif

class ISerializer {
    static inline constexpr const int SINGLE_LINE_MAX_ELEMENT = 4;

public:
    virtual ~ISerializer() = default;

    virtual ISerializer& beginArray(bool single_line) = 0;
    virtual ISerializer& endArray() = 0;

    virtual ISerializer& beginMap(bool single_line) = 0;
    virtual ISerializer& endMap() = 0;

    virtual ISerializer& beginKey(std::string_view key) = 0;

    virtual ISerializer& write(const bool& value) = 0;
    virtual ISerializer& write(const float& value) = 0;
    virtual ISerializer& write(const char* value) = 0;
    virtual ISerializer& write(std::string_view value) = 0;
    virtual ISerializer& write(const std::string& value) = 0;

    virtual ISerializer& write(const int8_t& value) = 0;
    virtual ISerializer& write(const uint8_t& value) = 0;
    virtual ISerializer& write(const int16_t& value) = 0;
    virtual ISerializer& write(const uint16_t& value) = 0;
    virtual ISerializer& write(const int32_t& value) = 0;
    virtual ISerializer& write(const uint32_t& value) = 0;
    virtual ISerializer& write(const int64_t& value) = 0;
    virtual ISerializer& write(const uint64_t& value) = 0;

    virtual ISerializer& write(const Guid& object) = 0;

    ISerializer& write(const ecs::Entity& object);
    ISerializer& write(const math::Degree& object);
    ISerializer& write(const math::Mat4f& object);
    ISerializer& write(const Variant& variant);

    template<size_t N>
    ISerializer& write(const FixedString<N>& value) {
        return write(value.data());
    }

    template<typename T, size_t N>
    ISerializer& write(const FixedStack<T, N>& array) {
        const size_t len = std::ranges::size(array);
        beginArray(len < SINGLE_LINE_MAX_ELEMENT);
        for (const T& val : array) write(val);
        endArray();
        return *this;
    }

    template<ArrayLike T>
    ISerializer& write(const T& array) {
        const size_t len = std::ranges::size(array);
        beginArray(len < SINGLE_LINE_MAX_ELEMENT);
        for (const auto& val : array) write(val);
        endArray();
        return *this;
    }

    template<StringKeyMap T>
    ISerializer& write(const T& map) {
        const size_t len = std::ranges::size(map);
        beginMap(len < SINGLE_LINE_MAX_ELEMENT);
        for (const auto& [key, value] : map) {
            beginKey(key).write(value);
        }
        endMap();
        return *this;
    }

    template<IntegralKeyMap T>
    ISerializer& write(const T& map) {
        const size_t len = std::ranges::size(map);
        beginMap(len < SINGLE_LINE_MAX_ELEMENT);
        for (const auto& [key, value] : map) {
            beginKey(std::to_string(key)).write(value);
        }
        endMap();
        return *this;
    }

    template<IsSerializable T>
    ISerializer& write(const T& value) {
        return WriteObject(*this, value);
    }

    template<IsEnum T>
    ISerializer& write(const T& object) {
        if constexpr (HasEnumTraits<T>) {
            return write(EnumTraits<T>::ToString(object).data());
        } else {
            return write(static_cast<uint64_t>(std::to_underlying(object)));
        }
    }

    template<typename T, int N>
    ISerializer& write(const math::Vec<T, N>& object) {
        beginArray(true);
        write(object.x);
        write(object.y);
        if constexpr (N > 2) {
            write(object.z);
        }
        if constexpr (N > 3) {
            write(object.w);
        }
        endArray();
        return *this;
    }

    template<typename T, int N>
    ISerializer& write(const T (&object)[N]) {
        beginArray(true);
        for (int i = 0; i < N; ++i) {
            write(object[i]);
        }
        endArray();
        return *this;
    }

    template<typename T, int N>
    ISerializer& write(const math::Box<T, N>& object) {
        beginMap(true)
            .beginKey("min")
            .write(object.min())
            .beginKey("max")
            .write(object.max());
        endMap();
        return *this;
    }

#if USING(USE_REFLECTION)
    template<IsReflectable T>
    ISerializer& write(const T& object) {
        const auto& meta = MetaDataTable<T>::getFields();

        beginMap(false);

        for (const FieldMetaBase& field : meta) {
            if ((field.flags & FieldFlag::Serialize) == FieldFlag::None) continue;
            field.write(*this, &object);
        }

        endMap();

        return *this;
    }
#endif

protected:
#if USING(VALIDATE_SERIALIZER)
    void checkEnter(SerializerState state);
    void checkExit(SerializerState state);

    Vector<SerializerState> m_stack;
#endif
};

}  // namespace cave