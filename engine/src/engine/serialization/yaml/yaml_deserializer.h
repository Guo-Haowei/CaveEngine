#pragma once
#include <yaml-cpp/yaml.h>

// @TODO: move it to core
#include "engine/serialization/concept.h"

#include "engine/ecs/entity.h"
#include "engine/math/angle.h"
#include "engine/math/box.h"
#include "engine/math/matrix.h"
#include "engine/reflection/meta.h"

namespace cave {

class Guid;
struct TileData;

auto LoadYaml(std::string_view p_path, YAML::Node& p_node) -> Result<void>;

class IDeserializer {
public:
    virtual ~IDeserializer() = default;

    virtual Option<int> GetVersion() const = 0;
};

class YamlDeserializer : public IDeserializer {
public:
    Option<int> GetVersion() const override;

    bool Deserialize(const YAML::Node& p_node, ecs::Entity& p_object);

    bool Deserialize(const YAML::Node& p_node, Degree& p_object);

    bool Deserialize(const YAML::Node& p_node, std::string& p_object);

    bool Deserialize(const YAML::Node& p_node, Guid& p_object);

    bool Deserialize(const YAML::Node& p_node, Matrix4x4f& p_object);

    bool Deserialize(const YAML::Node& p_node, const TileData& p_tile_data);

    template<IsArithmetic T>
    bool Deserialize(const YAML::Node& p_node, T& p_object) {
        ERR_FAIL_COND_V_MSG(!p_node.IsScalar(), false, "expect scalar");

        p_object = p_node.as<T>();
        return true;
    }

    template<IsEnum T>
    bool Deserialize(const YAML::Node& p_node, T& p_object) {
        ERR_FAIL_COND_V_MSG(!p_node.IsScalar(), false, "expect scalar");

        p_object = static_cast<T>(p_node.as<uint64_t>());
        return true;
    }

    template<typename T, int N>
    bool Deserialize(const YAML::Node& p_node, Vector<T, N>& p_object) {
        ERR_FAIL_COND_V_MSG(!p_node.IsSequence() || p_node.size() != N, false, "expect Vector");

        p_object.x = p_node[0].as<T>();
        p_object.y = p_node[1].as<T>();
        if constexpr (N > 2) {
            p_object.z = p_node[2].as<T>();
        }
        if constexpr (N > 3) {
            p_object.w = p_node[3].as<T>();
        }
        return true;
    }

    template<int N>
    bool Deserialize(const YAML::Node& p_node, Box<N>& p_object) {
        ERR_FAIL_COND_V_MSG(!p_node.IsMap(), false, "expect Box");

        Vector<float, N> min, max;
        const auto& min_node = p_node["min"];
        ERR_FAIL_COND_V_MSG(!min_node || !Deserialize(min_node, min), false, "Invalid Box.min");
        const auto& max_node = p_node["max"];
        ERR_FAIL_COND_V_MSG(!max_node || !Deserialize(max_node, max), false, "Invalid Box.min");

        p_object = Box<N>(min, max);
        return true;
    }

#if USING(USE_REFLECTION)
    template<HasMetaTag T>
    bool Deserialize(const YAML::Node& p_node, T& p_object) {
        const auto& meta = MetaDataTable<T>::GetFields();

        for (const auto& field : meta) {
            const bool field_ok = field->Read(*this, p_node, &p_object);
            ERR_FAIL_COND_V_MSG(!field_ok, false, field->name);
        }

        return true;
    }
#endif

public:
    YAML::Node m_node;
};

template<typename T>
bool FieldMeta<T>::Read(IDeserializer& p_deserializer, const YAML::Node& p_node, void* p_object) {
    unused(p_deserializer);
    unused(p_node);
    unused(p_object);
#if 0
    const auto& field = p_node[name];
    ERR_FAIL_COND_V_MSG(!field, false, "field missing");

    T& data = FieldMetaBase::GetData<T>(p_object);
    return p_deserializer.Deserialize(field, data);
#endif
    return false;
}

#if 0
template<typename T, int N>
Result<void> DeserializeYaml(const YAML::Node& p_node, T (&p_object)[N], YamlSerializer& p_serializer) {
    if (!p_node) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Not defined");
    }

    if (!p_node.IsSequence() || p_node.size() != N) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Expect sequence of {}", N);
    }

    for (int i = 0; i < N; ++i) {
        if (auto res = DeserializeYaml(p_node[i], p_object[i], p_serializer); !res) {
            return CAVE_ERROR(res.error());
        }
    }

    return Result<void>();
}

template<typename T>
Result<void> DeserializeYamlVec(const YAML::Node& p_node, std::vector<T>& p_object, YamlSerializer& p_serializer) {
    DEV_ASSERT(!(p_serializer.flags & FieldFlag::BINARY));
    if (!p_node || !p_node.IsSequence()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "not a valid sequence");
    }

    const size_t count = p_node.size();
    p_object.resize(count);
    for (size_t i = 0; i < count; ++i) {
        if (auto res = DeserializeYaml(p_node[i], p_object[i], p_serializer); !res) {
            return CAVE_ERROR(res.error());
        }
    }
    return Result<void>();
}

template<typename T>
Result<void> DeserializeYamlVecBinary(const YAML::Node& p_node, std::vector<T>& p_object, YamlSerializer& p_serializer) {
    DEV_ASSERT(p_serializer.flags & FieldFlag::BINARY);
    constexpr size_t element_size = sizeof(p_object[0]);
    constexpr size_t internal_offset = sizeof(BIN_GUARD_MAGIC) + sizeof(size_t);
    if (!p_node || !p_node.IsMap()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "not a valid buffer, expect (length, offset)");
    }

    auto& file = p_serializer.file;
    DEV_ASSERT(file);

    size_t offset = 0;
    size_t size_in_byte = 0;
    if (auto res = DeserializeYaml(p_node["offset"], offset, p_serializer); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = DeserializeYaml(p_node["buffer_size"], size_in_byte, p_serializer); !res) {
        return CAVE_ERROR(res.error());
    }
    if (size_in_byte == 0) {
        return Result<void>();
    }
    DEV_ASSERT(size_in_byte % element_size == 0);
    DEV_ASSERT(offset >= internal_offset);

    const size_t element_count = size_in_byte / element_size;

    if (auto seek = file->Seek((long)(offset - internal_offset)); seek != 0) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CANT_READ, "Seek failed");
    }

    char magic[sizeof(BIN_GUARD_MAGIC)];
    if (auto res = FileRead(file, magic); !res) {
        return CAVE_ERROR(res.error());
    }

    if (!StringUtils::StringEqual(magic, BIN_GUARD_MAGIC)) {
        magic[sizeof(BIN_GUARD_MAGIC) - 1] = 0;
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "wrong magic {}", magic);
    }

    size_t stored_length = 0;
    if (auto res = FileRead(file, stored_length); !res) {
        return CAVE_ERROR(res.error());
    }

    if (stored_length != size_in_byte) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "wrong size (cache: {})", stored_length);
    }

    p_object.resize(element_count);
    if (auto res = FileRead(file, p_object.data(), size_in_byte); !res) {
        return CAVE_ERROR(res.error());
    }

    return Result<void>();
}
#endif

}  // namespace cave
