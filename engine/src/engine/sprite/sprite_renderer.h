#pragma once
#include "engine/reflection/reflection.h"
#include "engine/assets/asset_handle.h"
#include "engine/math/box.h"
#include "engine/math/geomath.h"

namespace cave {

class Archive;

struct SpriteRenderer {
    CAVE_META(SpriteRenderer)

public:
    CAVE_PROP(type = guid)
    Guid image_id;

    CAVE_PROP(type = color)
    Vector4f color = Vector4f::Zero;

    CAVE_PROP(type = bound2d)
    Rect rect;

    CAVE_PROP(type = boolean, ui = toggle)
    bool flip_x = false;

    CAVE_PROP(type = boolean, ui = toggle)
    bool flip_y = false;

    CAVE_PROP(type = boolean)
    bool is_billboard = false;

    // Non serialized
    Handle<ImageAsset> image_handle;

    void OnDeserialized() {}
    void Serialize(Archive& p_archive, uint32_t p_version);
};

}  // namespace cave
