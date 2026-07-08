#pragma once
// @TODO: refactor defines.h
#include "defines.h"

#include "cave/core/math/Box.h"
#include "cave/core/math/Matrix.h"
#include "cave/core/containers/FixedStack.h"
#include "cave/core/containers/FixedString.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave::math {
class Degree;
}  // namespace cave::math

namespace cave {

class Guid;
class Variant;

class IDeserializer {
public:
    virtual ~IDeserializer() = default;

    virtual int version() const = 0;

    virtual bool tryEnterKey(const char* key) = 0;
    virtual void leaveKey() = 0;

    virtual bool tryEnterIndex(int index) = 0;
    virtual void leaveIndex() = 0;

    virtual Option<int> arraySize() = 0;

    virtual Option<std::vector<std::string>> getKeys();

    virtual bool read(bool& value) = 0;
    virtual bool read(float& value) = 0;
    virtual bool read(std::string& value) = 0;

    virtual bool read(int8_t& value) = 0;
    virtual bool read(uint8_t& value) = 0;
    virtual bool read(int16_t& value) = 0;
    virtual bool read(uint16_t& value) = 0;
    virtual bool read(int32_t& value) = 0;
    virtual bool read(uint32_t& value) = 0;
    virtual bool read(int64_t& value) = 0;
    virtual bool read(uint64_t& value) = 0;

    bool read(ecs::Entity& object);
    bool read(math::Degree& object);
    bool read(Guid& object);
    bool read(math::Mat4f& object);
    bool read(Variant& variant);

    template<size_t N>
    bool read(FixedString<N>& value) {
        std::string s;
        if (!read(s)) {
            return false;
        }
        value = s;
        return true;
    }

    template<typename T, size_t N>
    bool read(FixedStack<T, N>& array) {
        const auto size = arraySize().unwrap_or(-1);
        ERR_FAIL_COND_V_MSG(size < 0, false, "expect array[]");
        ERR_FAIL_COND_V_MSG(size > array.capacity(), false, "array overflow");

        array.resize(size);
        for (int i = 0; i < size; ++i) {
            tryEnterIndex(i);
            read(array[i]);
            leaveIndex();
        }

        return true;
    }

    template<IsSerializable T>
    bool read(T& value) {
        return ReadObject(*this, value);
    }

    template<IsEnum T>
    bool read(T& object) {
        if constexpr (HasEnumTraits<T>) {
            std::string value;
            read(value);
            object = EnumTraits<T>::FromString(value).unwrap_or(static_cast<T>(0));
        } else {
            uint64_t value = 0;
            read(value);
            object = static_cast<T>(value);
        }
        return true;
    }

    template<ArrayLike T>
    bool read(T& array) {
        const auto size = arraySize().unwrap_or(-1);
        ERR_FAIL_COND_V_MSG(size < 0, false, "expect array[]");

        if constexpr (HasResize<T>) {
            array.resize(size);
        }
        for (int i = 0; i < size; ++i) {
            tryEnterIndex(i);
            read(array[i]);
            leaveIndex();
        }

        return true;
    }

    template<StringKeyMap T>
    bool read(T& map) {
        if (auto _keys = getKeys()) {
            for (const auto& key : _keys.unwrap_unchecked()) {
                if (tryEnterKey(key.c_str())) {
                    read(map[key]);
                    leaveKey();
                }
            }

            return true;
        }

        return false;
    }

    template<IntegralKeyMap T>
    bool read(T& map) {
        if (auto _keys = getKeys()) {
            for (const auto& key : _keys.unwrap_unchecked()) {
                tryEnterKey(key.c_str());
                const auto val = static_cast<typename MapTraits<T>::key_type>(std::stoll(key));
                read(map[val]);
                leaveKey();
            }

            return true;
        }

        return false;
    }

    template<typename T, int N>
    bool read(math::Vec<T, N>& object) {
        const auto size = arraySize().unwrap_or(-1);
        ERR_FAIL_COND_V_MSG(size != N, false, "expect vector");

        for (int i = 0; i < size; ++i) {
            DEV_ASSERT(tryEnterIndex(i));
            read(object[i]);
            leaveIndex();
        }

        return true;
    }

    template<typename T, int N>
    bool read(math::Box<T, N>& object) {
        auto min = math::Vec<T, N>(std::numeric_limits<T>::infinity());
        if (tryEnterKey("min")) {
            read(min);
            leaveKey();
        }

        auto max = math::Vec<T, N>(-std::numeric_limits<T>::infinity());
        if (tryEnterKey("max")) {
            read(max);
            leaveKey();
        }

        object = math::Box<T, N>(min, max);
        return true;
    }

#if USING(USE_REFLECTION)
    template<IsReflectable T>
    bool read(T& object) {
        const auto& meta = MetaDataTable<T>::GetFields();

        for (const auto& field : meta) {
            if ((field->flags & FieldFlag::Serialize) == FieldFlag::None) continue;
            if (tryEnterKey(field->name)) {
                field->Read(*this, &object);
                leaveKey();
            }
        }

        return true;
    }
#endif
};

}  // namespace cave
