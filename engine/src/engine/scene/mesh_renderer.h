#pragma once
#include "engine/ecs/entity.h"
#include "engine/reflection/reflection.h"

namespace cave {

class Archive;

struct MeshRenderer {
    CAVE_META(MeshRenderer)

public:
    enum : uint32_t {
        FLAG_RENDERABLE = BIT(1),
        FLAG_CAST_SHADOW = BIT(2),
        FLAG_TRANSPARENT = BIT(3),
    };

    CAVE_PROP(type = u32)
    uint32_t flags = FLAG_RENDERABLE | FLAG_CAST_SHADOW;

    // @TODO: make flag bools so it's esier to set in editor
    ecs::Entity meshId;

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
};

}  // namespace cave
