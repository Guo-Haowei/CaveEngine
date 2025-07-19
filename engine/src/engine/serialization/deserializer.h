#pragma once
#include "defines.h"
#include "engine/ecs/entity.h"
#include "engine/math/box.h"
#include "engine/math/matrix.h"

namespace cave {

class Degree;
class Guid;

class IDeserializer {
public:
    virtual ~IDeserializer() = default;

    virtual int GetVersion() const = 0;

    virtual bool TryEnterKey(const char* p_key) = 0;

    virtual void LeaveKey() = 0;

    virtual bool TryEnterIndex(int p_index) = 0;

    virtual void LeaveIndex() = 0;

    virtual Option<int> ArraySize() = 0;

    virtual Option<std::vector<std::string>> GetKeys();

    virtual bool Read(bool& p_value) = 0;
    virtual bool Read(float& p_value) = 0;
    virtual bool Read(std::string& p_value) = 0;

    virtual bool Read(int8_t& p_value) = 0;
    virtual bool Read(uint8_t& p_value) = 0;
    virtual bool Read(int16_t& p_value) = 0;
    virtual bool Read(uint16_t& p_value) = 0;
    virtual bool Read(int32_t& p_value) = 0;
    virtual bool Read(uint32_t& p_value) = 0;
    virtual bool Read(int64_t& p_value) = 0;
    virtual bool Read(uint64_t& p_value) = 0;

    bool Read(ecs::Entity& p_object);
    bool Read(Degree& p_object);
    bool Read(Guid& p_object);
    bool Read(Matrix4x4f& p_object);

    template<IsSerializable T>
    bool Read(T& p_value) {
        return ReadObject(*this, p_value);
    }

    template<IsEnum T>
    bool Read(T& p_object) {
        uint64_t value = 0;
        Read(value);
        p_object = static_cast<T>(value);
        return true;
    }

    template<ArrayLike T>
    bool Read(T& p_array) {
        const auto size = ArraySize().unwrap_or(-1);
        ERR_FAIL_COND_V_MSG(size < 0, false, "expect array[]");

        for (int i = 0; i < size; ++i) {
            TryEnterIndex(i);
            Read(p_array[i]);
            LeaveIndex();
        }

        return true;
    }

    template<StringMap T>
    bool Read(T& p_map) {
        if (auto _keys = GetKeys(); _keys.is_some()) {
            for (const auto& key : _keys.unwrap_unchecked()) {
                TryEnterKey(key.c_str());
                Read(p_map[key]);
                LeaveKey();
            }

            return true;
        }

        return false;
    }

    template<typename T, int N>
    bool Read(Vector<T, N>& p_object) {
        const auto size = ArraySize().unwrap_or(-1);
        ERR_FAIL_COND_V_MSG(size != N, false, "expect vector");

        for (int i = 0; i < size; ++i) {
            DEV_ASSERT(TryEnterIndex(i));
            Read(p_object[i]);
            LeaveIndex();
        }

        return true;
    }

    template<int N>
    bool Read(Box<N>& p_object) {
        auto min = Vector<float, N>(std::numeric_limits<float>::infinity());
        if (TryEnterKey("min")) {
            Read(min);
            LeaveKey();
        }

        auto max = Vector<float, N>(-std::numeric_limits<float>::infinity());
        if (TryEnterKey("max")) {
            Read(max);
            LeaveKey();
        }

        p_object = Box<N>(min, max);
        return true;
    }

#if USING(USE_REFLECTION)
    template<IsReflectable T>
    bool Read(T& p_object) {
        const auto& meta = MetaDataTable<T>::GetFields();

        for (const auto& field : meta) {
            if (TryEnterKey(field->name)) {
                field->Read(*this, &p_object);
                LeaveKey();
            }
        }

        return true;
    }
#endif
};

}  // namespace cave
