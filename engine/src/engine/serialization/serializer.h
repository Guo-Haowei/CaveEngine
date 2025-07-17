#pragma once
// @TODO: move it to core
#include "engine/serialization/concept.h"

#include "engine/core/io/file_access.h"
#include "engine/core/string/string_utils.h"
#include "engine/ecs/entity.h"
#include "engine/math/angle.h"
#include "engine/math/box.h"
#include "engine/math/matrix.h"

#include "engine/reflection/meta.h"

namespace cave {

enum FieldFlag : uint32_t {
    NONE = BIT(0),
};

DEFINE_ENUM_BITWISE_OPERATIONS(FieldFlag);

class Guid;

class ISerializer {
public:
    virtual ~ISerializer() = default;

    // virtual bool IsGood() const = 0;

    // virtual auto WriteToFile() -> Result<void> = 0;

    // virtual const std::string& GetError() const = 0;

    // virtual const std::string& GetWarning() const = 0;

    virtual ISerializer& BeginArray(bool p_single_line = false) = 0;
    virtual ISerializer& EndArray() = 0;

    virtual ISerializer& BeginMap(bool p_single_line = false) = 0;
    virtual ISerializer& EndMap() = 0;

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
    ISerializer& Write(const Degree& p_object);
    ISerializer& Write(const Matrix4x4f& p_object);

    template<IsSerializable T>
    ISerializer& Write(const T& p_value) {
        return WriteObject(*this, p_value);
    }

    template<std::ranges::range T>
    ISerializer& Write(const T& p_container) {
        const size_t len = std::ranges::size(p_container);
        BeginArray(len < 4);
        for (const auto& val : p_container) {
            Write(val);
        }
        EndArray();
        return *this;
    }

    template<IsEnum T>
    ISerializer& Write(const T& p_object) {
        return Write(static_cast<uint64_t>(std::to_underlying(p_object)));
    }

    template<typename T, int N>
    ISerializer& Write(const Vector<T, N>& p_object) {
        BeginArray(true);
        Write(p_object.x);
        Write(p_object.y);
        if constexpr (N > 2) {
            Write(p_object.z);
        }
        if constexpr (N > 3) {
            Write(p_object.w);
        }
        EndArray();
        return *this;
    }

    template<typename T, int N>
    ISerializer& Write(const T (&p_object)[N]) {
        BeginArray();
        for (int i = 0; i < N; ++i) {
            Write(p_object[i]);
        }
        EndArray();
        return *this;
    }

    template<int N>
    ISerializer& Write(const Box<N>& p_object) {
        BeginMap(true)
            .Key("min")
            .Write(p_object.GetMin())
            .Key("max")
            .Write(p_object.GetMax());
        EndMap();
        return *this;
    }

#if USING(USE_REFLECTION)
    template<HasMetaTag T>
    ISerializer& Write(const T& p_object) {
        const auto& meta = MetaDataTable<T>::GetFields();

        BeginMap();

        for (const auto& field : meta) {
            field->Write(*this, &p_object);
        }

        EndMap();

        return *this;
    }
#endif
};

template<typename T>
ISerializer& FieldMeta<T>::Write(ISerializer& p_serializer, const void* p_object) const {
    const T& data = FieldMetaBase::GetData<T>(p_object);
    return p_serializer.Key(name).Write(data);
}

}  // namespace cave