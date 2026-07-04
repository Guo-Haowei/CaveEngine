#pragma once
// @TODO: refactor defines.h
#include "defines.h"

#include "cave/core/math/Angle.h"
#include "cave/core/math/Box.h"
#include "cave/core/math/Matrix.h"
#include "cave/core/containers/FixedStack.h"
#include "cave/core/containers/FixedString.h"
#include "cave/runtime/ecs/Entity.h"

#include "engine/private/core/io/file_access.h"

namespace cave {

class Guid;

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

    virtual ISerializer& Key(std::string_view p_key) = 0;

    virtual ISerializer& Write(const bool& p_value) = 0;
    virtual ISerializer& Write(const float& p_value) = 0;
    virtual ISerializer& Write(const char* p_value) = 0;
    virtual ISerializer& Write(const std::string& p_value) = 0;

    virtual ISerializer& Write(const int8_t& p_value) = 0;
    virtual ISerializer& Write(const uint8_t& p_value) = 0;
    virtual ISerializer& Write(const int16_t& p_value) = 0;
    virtual ISerializer& Write(const uint16_t& p_value) = 0;
    virtual ISerializer& Write(const int32_t& p_value) = 0;
    virtual ISerializer& Write(const uint32_t& p_value) = 0;
    virtual ISerializer& Write(const int64_t& p_value) = 0;
    virtual ISerializer& Write(const uint64_t& p_value) = 0;

    virtual ISerializer& Write(const Guid& p_object) = 0;

    ISerializer& Write(const ecs::Entity& p_object);
    ISerializer& Write(const math::Degree& p_object);
    ISerializer& Write(const math::Mat4f& p_object);

    template<size_t N>
    ISerializer& Write(const FixedString<N>& p_value) {
        return Write(p_value.data());
    }

    template<typename T, size_t N>
    ISerializer& Write(const FixedStack<T, N>& p_array) {
        const size_t len = std::ranges::size(p_array);
        beginArray(len < SINGLE_LINE_MAX_ELEMENT);
        for (const T& val : p_array) Write(val);
        endArray();
        return *this;
    }

    template<ArrayLike T>
    ISerializer& Write(const T& p_array) {
        const size_t len = std::ranges::size(p_array);
        beginArray(len < SINGLE_LINE_MAX_ELEMENT);
        for (const auto& val : p_array) Write(val);
        endArray();
        return *this;
    }

    template<StringKeyMap T>
    ISerializer& Write(const T& p_map) {
        const size_t len = std::ranges::size(p_map);
        beginMap(len < SINGLE_LINE_MAX_ELEMENT);
        for (const auto& [key, value] : p_map) {
            Key(key).Write(value);
        }
        endMap();
        return *this;
    }

    template<IntegralKeyMap T>
    ISerializer& Write(const T& p_map) {
        const size_t len = std::ranges::size(p_map);
        beginMap(len < SINGLE_LINE_MAX_ELEMENT);
        for (const auto& [key, value] : p_map) {
            Key(std::to_string(key)).Write(value);
        }
        endMap();
        return *this;
    }

    template<IsSerializable T>
    ISerializer& Write(const T& p_value) {
        return WriteObject(*this, p_value);
    }

    template<IsEnum T>
    ISerializer& Write(const T& p_object) {
        if constexpr (HasEnumTraits<T>) {
            return Write(EnumTraits<T>::ToString(p_object).data());
        } else {
            return Write(static_cast<uint64_t>(std::to_underlying(p_object)));
        }
    }

    template<typename T, int N>
    ISerializer& Write(const math::Vector<T, N>& p_object) {
        beginArray(true);
        Write(p_object.x);
        Write(p_object.y);
        if constexpr (N > 2) {
            Write(p_object.z);
        }
        if constexpr (N > 3) {
            Write(p_object.w);
        }
        endArray();
        return *this;
    }

    template<typename T, int N>
    ISerializer& Write(const T (&p_object)[N]) {
        beginArray(true);
        for (int i = 0; i < N; ++i) {
            Write(p_object[i]);
        }
        endArray();
        return *this;
    }

    template<typename T, int N>
    ISerializer& Write(const math::Box<T, N>& p_object) {
        beginMap(true)
            .Key("min")
            .Write(p_object.min())
            .Key("max")
            .Write(p_object.max());
        endMap();
        return *this;
    }

#if USING(USE_REFLECTION)
    template<IsReflectable T>
    ISerializer& Write(const T& p_object) {
        const auto& meta = MetaDataTable<T>::GetFields();

        beginMap(false);

        for (const auto& field : meta) {
            if ((field->flags & FieldFlag::Serialize) == FieldFlag::None) continue;
            field->Write(*this, &p_object);
        }

        endMap();

        return *this;
    }
#endif

protected:
#if USING(VALIDATE_SERIALIZER)
    void CheckEnter(SerializerState p_state);
    void CheckExit(SerializerState p_state);

    std::vector<SerializerState> m_stack;
#endif
};

template<typename T>
ISerializer& FieldMeta<T>::Write(ISerializer& p_serializer, const void* p_object) const {
    const T& data = FieldMetaBase::GetData<T>(p_object);
    return p_serializer.Key(name).Write(data);
}

}  // namespace cave