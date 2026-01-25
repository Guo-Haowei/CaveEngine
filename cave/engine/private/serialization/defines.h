#pragma once
#include "engine/private/reflection/meta.h"
#include "engine/private/serialization/concept.h"

#define VALIDATE_SERIALIZER USE_IF(USING(ENABLE_ASSERT))

namespace cave {

class ISerializer;
class IDeserializer;

enum class SerializerState {
    Array,
    Map,
};

}  // namespace cave
