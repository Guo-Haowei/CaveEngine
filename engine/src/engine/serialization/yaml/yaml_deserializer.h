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

    virtual int GetVersion() const = 0;
};

class YamlDeserializer : public IDeserializer {
public:
    // @TODO: make it private
    bool Initialize(const YAML::Node& p_node);

    int GetVersion() const override {
        DEV_ASSERT(m_initialized);
        return m_version;
    }

    bool TryEnterKey(const char* p_key);
    void LeaveKey();

    bool Read(ecs::Entity& p_object);

    bool Read(Degree& p_object);

    bool Read(std::string& p_object);

    bool Read(Guid& p_object);

    bool Read(Matrix4x4f& p_object);

    bool Read(const TileData& p_tile_data);

    template<IsArithmetic T>
    bool Read(T& p_object) {
        auto& node = Current();

        ERR_FAIL_COND_V_MSG(!node.IsScalar(), false, "expect scalar");

        p_object = node.as<T>();
        return true;
    }

    template<IsEnum T>
    bool Read(T& p_object) {
        auto& node = Current();

        ERR_FAIL_COND_V_MSG(!node.IsScalar(), false, "expect scalar");

        p_object = static_cast<T>(node.as<uint64_t>());
        return true;
    }

    template<typename T, int N>
    bool Read(Vector<T, N>& p_object) {
        auto& node = Current();

        ERR_FAIL_COND_V_MSG(!node.IsSequence() || node.size() != N, false, "expect Vector");

        p_object.x = node[0].as<T>();
        p_object.y = node[1].as<T>();
        if constexpr (N > 2) {
            p_object.z = node[2].as<T>();
        }
        if constexpr (N > 3) {
            p_object.w = node[3].as<T>();
        }
        return true;
    }

    template<int N>
    bool Read(Box<N>& p_object) {
        ERR_FAIL_COND_V_MSG(!Current().IsMap(), false, "expect Box<N>");

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
    template<HasMetaTag T>
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

private:
    const YAML::Node& Current() {
        DEV_ASSERT(!m_stack.empty());
        return m_stack.back();
    }

    std::vector<YAML::Node> m_stack;
    int m_version{ -1 };
    bool m_initialized{ false };
};

template<typename T>
bool FieldMeta<T>::Read(IDeserializer& p_deserializer, void* p_object) {
    T& data = FieldMetaBase::GetData<T>(p_object);

    return static_cast<YamlDeserializer&>(p_deserializer).Read(data);
}

#if 0
template<typename T, int N>
Result<void> DeserializeYaml(const YAML::Node& node, T (&p_object)[N], YamlSerializer& p_serializer) {
    if (!node) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Not defined");
    }

    if (!node.IsSequence() || node.size() != N) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Expect sequence of {}", N);
    }

    for (int i = 0; i < N; ++i) {
        if (auto res = DeserializeYaml(node[i], p_object[i], p_serializer); !res) {
            return CAVE_ERROR(res.error());
        }
    }

    return Result<void>();
}

template<typename T>
Result<void> DeserializeYamlVec(const YAML::Node& node, std::vector<T>& p_object, YamlSerializer& p_serializer) {
    DEV_ASSERT(!(p_serializer.flags & FieldFlag::BINARY));
    if (!node || !node.IsSequence()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "not a valid sequence");
    }

    const size_t count = node.size();
    p_object.resize(count);
    for (size_t i = 0; i < count; ++i) {
        if (auto res = DeserializeYaml(node[i], p_object[i], p_serializer); !res) {
            return CAVE_ERROR(res.error());
        }
    }
    return Result<void>();
}

template<typename T>
Result<void> DeserializeYamlVecBinary(const YAML::Node& node, std::vector<T>& p_object, YamlSerializer& p_serializer) {
    DEV_ASSERT(p_serializer.flags & FieldFlag::BINARY);
    constexpr size_t element_size = sizeof(p_object[0]);
    constexpr size_t internal_offset = sizeof(BIN_GUARD_MAGIC) + sizeof(size_t);
    if (!node || !node.IsMap()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "not a valid buffer, expect (length, offset)");
    }

    auto& file = p_serializer.file;
    DEV_ASSERT(file);

    size_t offset = 0;
    size_t size_in_byte = 0;
    if (auto res = DeserializeYaml(node["offset"], offset, p_serializer); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = DeserializeYaml(node["buffer_size"], size_in_byte, p_serializer); !res) {
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
