#pragma once
#include <yaml-cpp/yaml.h>

#include "engine/core/io/file_access.h"
#include "engine/core/string/string_utils.h"
#include "engine/ecs/entity.h"
#include "engine/math/angle.h"
#include "engine/math/box.h"
#include "engine/math/matrix.h"
#include "engine/reflection/meta.h"

#include "engine/tile_map/tile_map_asset.h"

namespace cave {

class FileAccess;

enum FieldFlag : uint32_t {
    NONE = BIT(0),
    NUALLABLE = BIT(1),
    BINARY = BIT(2),
    EMIT_SAME_LINE = BIT(3),
};

DEFINE_ENUM_BITWISE_OPERATIONS(FieldFlag);

class Guid;

template<typename T>
concept IsArithmetic = std::is_arithmetic_v<T>;

template<typename T>
concept IsEnum = std::is_enum_v<T>;

class YamlSerializer {
public:
    YamlSerializer(YAML::Emitter& p_emitter)
        : m_out(p_emitter) {}

    YamlSerializer& BeginArray(bool p_single_line = false);
    YamlSerializer& EndArray();

    YamlSerializer& BeginMap(bool p_single_line = false);
    YamlSerializer& EndMap();

    template<typename T>
    YamlSerializer& Value(const T& p_value) {
        Serialize(p_value);
        return *this;
    }

    template<typename T>
    YamlSerializer& KeyValue(const char* p_key, const T& p_value) {
        m_out << ::YAML::Key << p_key << ::YAML::Value;
        Serialize(p_value);
        return *this;
    }

    void Serialize(const Guid& p_object);
    void Serialize(const std::string& p_object);
    void Serialize(const ecs::Entity& p_object);
    void Serialize(const Degree& p_object);

    void Serialize(const Matrix4x4f& p_object);

    void Serialize(const TileData& p_tile_data);

    template<IsArithmetic T>
    void Serialize(const T& p_object) {
        m_out << p_object;
    }

    template<IsEnum T>
    void Serialize(const T& p_object) {
        m_out << std::to_underlying(p_object);
    }

    template<typename T, int N>
    void Serialize(const T (&p_object)[N]) {
        BeginArray();
        for (int i = 0; i < N; ++i) {
            Serialize(p_object[i]);
        }
        EndArray();
    }

    // @TODO: for iterable
    template<typename T, int N>
    void Serialize(const Vector<T, N>& p_object) {
        BeginArray(true);
        Value(p_object.x);
        Value(p_object.y);
        if constexpr (N > 2) {
            Value(p_object.z);
        }
        if constexpr (N > 3) {
            Value(p_object.w);
        }
        EndArray();
    }

#if 0
    template<typename T>
    Result<void> SerializaYamlVec(YAML::Emitter& p_out, const std::vector<T>& p_object, YamlSerializer& p_serializer) {
        DEV_ASSERT(!(p_serializer.flags & FieldFlag::BINARY));
        const size_t count = p_object.size();
        if (count < 4) {
            p_out.SetSeqFormat(YAML::Flow);
        }
        p_out << YAML::BeginSeq;
        for (size_t i = 0; i < count; ++i) {
            if (auto res = SerializeYaml(p_out, p_object[i], p_serializer); !res) {
                return CAVE_ERROR(res.error());
            }
        }
        p_out << YAML::EndSeq;
        p_out.SetSeqFormat(YAML::Block);
        return Result<void>();
    }
#endif

    template<int N>
    void Serialize(const Box<N>& p_object) {
        if (!p_object.IsValid()) {
            m_out << YAML::Null;
            return;
        }

        BeginMap(true)
            .KeyValue("min", p_object.GetMin())
            .KeyValue("max", p_object.GetMax());
        EndMap();
    }

#if USING(USE_REFLECTION)
    template<HasMetaTag T>
    void Serialize(const T& p_object) {
        const auto& meta = MetaDataTable<T>::GetFields();

        BeginMap();

        for (const auto& field : meta) {
            if (auto res = field->DumpYaml(m_out, &p_object, *this); !res) {
                // return CAVE_ERROR(res.error());
            }
        }

        EndMap();
    }
#endif

public:
    FieldFlag flags;
    uint32_t version;
    FileAccess* file;

    void PushWarning(std::string&& p_warning);

    void PushError(std::string&& p_error);

    YAML::Emitter& m_out;

private:
    std::vector<std::string> m_errors;
    std::vector<std::string> m_warnings;
};

}  // namespace cave

#define DUMP_KEY(a) ::YAML::Key << (a) << ::YAML::Value

namespace cave {

Result<void> DeserializeYaml(const YAML::Node& p_node, ecs::Entity& p_object, YamlSerializer&);

Result<void> DeserializeYaml(const YAML::Node& p_node, Degree& p_object, YamlSerializer&);

Result<void> DeserializeYaml(const YAML::Node& p_node, std::string& p_object, YamlSerializer&);

Result<void> DeserializeYaml(const YAML::Node& p_node, Guid& p_object, YamlSerializer&);

inline Result<void> DeserializeYaml(const YAML::Node& p_node, Matrix4x4f& p_object, YamlSerializer&) {
    if (!p_node || !p_node.IsSequence() || p_node.size() != 16) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "not a Matrix4x4f");
    }

    float* ptr = &p_object[0].x;
    for (int i = 0; i < 16; ++i) {
        ptr[i] = p_node[i].as<float>();
    }
    return Result<void>();
}

template<IsArithmetic T>
Result<void> DeserializeYaml(const YAML::Node& p_node, T& p_object, YamlSerializer&) {
    if (!p_node) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Not defined");
    }

    if (!p_node.IsScalar()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Expect scalar");
    }

    p_object = p_node.as<T>();
    return Result<void>();
}

template<IsEnum T>
Result<void> DeserializeYaml(const YAML::Node& p_node, T& p_object, YamlSerializer&) {
    if (!p_node) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Not defined");
    }

    if (!p_node.IsScalar()) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Expect scalar");
    }

    p_object = static_cast<T>(p_node.as<uint64_t>());
    return Result<void>();
}

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

template<typename T, int N>
Result<void> DeserializeYaml(const YAML::Node& p_node, Vector<T, N>& p_object, YamlSerializer&) {
    if (!p_node) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Not defined");
    }

    if (!p_node.IsSequence() || p_node.size() != N) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Expect sequence of {}", N);
    }

    p_object.x = p_node[0].as<T>();
    p_object.y = p_node[1].as<T>();
    if constexpr (N > 2) {
        p_object.z = p_node[2].as<T>();
    }
    if constexpr (N > 3) {
        p_object.w = p_node[3].as<T>();
    }
    return Result<void>();
}

template<int N>
Result<void> DeserializeYaml(const YAML::Node& p_node, Box<N>& p_object, YamlSerializer& p_serializer) {
    if (!p_node || p_node.IsNull()) {
        p_object.MakeInvalid();
        return Result<void>();
    }

    Vector<float, N> min, max;
    if (auto res = DeserializeYaml(p_node["min"], min, p_serializer); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = DeserializeYaml(p_node["max"], max, p_serializer); !res) {
        return CAVE_ERROR(res.error());
    }

    p_object = AABB(min, max);
    return Result<void>();
}

template<HasMetaTag T>
Result<void> DeserializeYaml(const YAML::Node& p_node, T& p_object, YamlSerializer& p_serializer) {
    const auto& meta = MetaDataTable<T>::GetFields();

    for (const auto& field : meta) {
        if (auto res = field->UndumpYaml(p_node, &p_object, p_serializer); !res) {
            return CAVE_ERROR(res.error());
        }
    }
    return Result<void>();
}

template<typename T>
Result<void> FieldMeta<T>::DumpYaml(YAML::Emitter&, const void* p_object, YamlSerializer& p_serializer) const {
    const T& data = FieldMetaBase::GetData<T>(p_object);
    p_serializer.KeyValue(name, data);
    return Result<void>();
}

template<typename T>
Result<void> FieldMeta<T>::UndumpYaml(const YAML::Node& p_node, void* p_object, YamlSerializer&) {
    const auto& field = p_node[name];
    const bool nuallable = flags & FieldFlag::NUALLABLE;
    if (!field && !nuallable) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "missing '{}'", name);
    }
    if (nuallable && !field) {
        return Result<void>();
    }

    unused(p_object);

    //T& data = FieldMetaBase::GetData<T>(p_object);
    //return DeserializeYaml(field, data, p_serializer);
    return Result<void>();
}

static constexpr char BIN_GUARD_MAGIC[] = "SEETHIS";

static inline Result<void> FileWrite(FileAccess* p_file, const void* p_data, size_t p_length) {
    const size_t written = p_file->WriteBuffer(p_data, p_length);
    if (written != p_length) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CANT_WRITE, "failed to write {} bytes, only wrote {}", p_length, written);
    }
    return Result<void>();
}

template<TriviallyCopyable T>
static inline Result<void> FileWrite(FileAccess* p_file, const T& p_data) {
    return FileWrite(p_file, &p_data, sizeof(T));
}

static inline Result<void> FileRead(FileAccess* p_file, void* p_data, size_t p_length) {
    const size_t read = p_file->ReadBuffer(p_data, p_length);
    if (read != p_length) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CANT_READ, "failed to read {} bytes, only read {}", p_length, read);
    }
    return Result<void>();
}

template<TriviallyCopyable T>
static inline Result<void> FileRead(FileAccess* p_file, T& p_data) {
    return FileRead(p_file, &p_data, sizeof(T));
}

#if 0
template<typename T>
Result<void> SerializaYamlVecBinary(YAML::Emitter& p_out, const std::vector<T>& p_object, YamlSerializer& p_serializer) {
    DEV_ASSERT(p_serializer.flags & FieldFlag::BINARY);
    auto& file = p_serializer.file;
    DEV_ASSERT(file);

    const size_t count = p_object.size();
    const size_t size_in_byte = sizeof(p_object[0]) * count;

    long offset = 0;
    if (size_in_byte > 0) {
        if (auto res = FileWrite(file, BIN_GUARD_MAGIC); !res) {
            return CAVE_ERROR(res.error());
        }
        if (auto res = FileWrite(file, size_in_byte); !res) {
            return CAVE_ERROR(res.error());
        }
        // @TODO: better file API
        offset = file->Tell();
        DEV_ASSERT(offset > 0);
        if (auto res = FileWrite(file, p_object.data(), size_in_byte); !res) {
            return CAVE_ERROR(res.error());
        }
    }

    p_out.SetMapFormat(YAML::Flow);
    p_out << YAML::BeginMap;
    p_out << YAML::Key << "offset" << YAML::Value << offset;
    p_out << YAML::Key << "buffer_size" << YAML::Value << size_in_byte;
    p_out << YAML::EndMap;
    p_out.SetMapFormat(YAML::Block);
    return Result<void>();
}

template<typename T>
Result<void> SerializeYaml(YAML::Emitter& p_out, const std::vector<T>& p_object, YamlSerializer& p_serializer) {
    return (p_serializer.flags & FieldFlag::BINARY) ? SerializaYamlVecBinary(p_out, p_object, p_serializer) : SerializaYamlVec(p_out, p_object, p_serializer);
}

#endif

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

template<typename T>
Result<void> DeserializeYaml(const YAML::Node& p_node, std::vector<T>& p_object, YamlSerializer& p_serializer) {
    return (p_serializer.flags & FieldFlag::BINARY) ? DeserializeYamlVecBinary(p_node, p_object, p_serializer) : DeserializeYamlVec(p_node, p_object, p_serializer);
}

}  // namespace cave

namespace cave {

// @TODO: write to .tmp then rename, because renaming it atomic
auto SaveYaml(std::string_view p_path, YAML::Emitter& p_out) -> Result<void>;

auto LoadYaml(std::string_view p_path, YAML::Node& p_node) -> Result<void>;

}  // namespace cave
