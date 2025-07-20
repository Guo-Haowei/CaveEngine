#pragma once
#include "engine/reflection/reflection.h"
#include "engine/assets/asset_handle.h"
#include "engine/math/box.h"
#include "engine/math/geomath.h"

namespace cave {

class Archive;

struct SpriteRenderer {
    CAVE_META(SpriteRenderer)

    // @TODO: make property private for safety
public:
    CAVE_PROP(type = guid)
    Guid image_id;

    CAVE_PROP(type = color)
    Vector4f tint_color = Vector4f::One;

    CAVE_PROP(type = box2)
    Rect rect = { Vector2f::Zero, Vector2f::One };

    CAVE_PROP(type = boolean, ui = toggle)
    bool flip_x = false;

    CAVE_PROP(type = boolean, ui = toggle)
    bool flip_y = false;

    CAVE_PROP(type = boolean)
    bool is_billboard = false;

private:
    // Non serialized
    Handle<ImageAsset> m_image_handle;

public:
    bool SetImage(const Guid& p_guid);

    const Guid& GetGuid() const { return image_id; }

    const Handle<ImageAsset> GetHandle() const { return m_image_handle; }

    void OnDeserialized() {}
    void Serialize(Archive& p_archive, uint32_t p_version);
};

}  // namespace cave
