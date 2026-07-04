#pragma once
#include "cave/core/reflection/Meta.h"
#include "cave/runtime/serialization/Concepts.h"

#define VALIDATE_SERIALIZER USE_IF(USING(ENABLE_ASSERT))

namespace cave {

class ISerializer;
class IDeserializer;

enum class SerializerState {
    Array,
    Map,
};

}  // namespace cave
