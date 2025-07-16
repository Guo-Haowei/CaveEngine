#pragma once
#include "engine/reflection/meta.h"

namespace cave {

class FileAccess;

enum FieldFlag : uint32_t {
    NONE = BIT(0),
    NUALLABLE = BIT(1),
    BINARY = BIT(2),
    EMIT_SAME_LINE = BIT(3),
};

DEFINE_ENUM_BITWISE_OPERATIONS(FieldFlag);

struct SerializeYamlContext {
    FieldFlag flags;
    uint32_t version;
    FileAccess* file;
};

}  // namespace cave

#define DUMP_KEY(a) ::YAML::Key << (a) << ::YAML::Value
