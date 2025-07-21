#pragma once
#include "engine/reflection/reflection.h"
#include "engine/assets/asset_handle.h"
#include "engine/math/box.h"
#include "engine/math/geomath.h"

namespace cave {

class Archive;

class SpriteRenderer {
    CAVE_META(SpriteRenderer)

    CAVE_PROP(editor = Asset)
    Guid m_image_id;

    // @TODO: make property private
public:
    CAVE_PROP(editor = Color)
    Vector4f tint_color = Vector4f::One;

    CAVE_PROP()
    Rect rect = { Vector2f::Zero, Vector2f::One };

    CAVE_PROP(editor = Toggle)
    bool flip_x = false;

    CAVE_PROP(editor = Toggle)
    bool flip_y = false;

    CAVE_PROP(editor = Toggle)
    bool is_billboard = false;

private:
    // Non serialized
    Handle<ImageAsset> m_image_handle;

public:
    bool SetResourceGuid(const Guid& p_guid);

    const Guid& GetResourceGuid() const { return m_image_id; }

    const Handle<ImageAsset> GetHandle() const { return m_image_handle; }

    void OnDeserialized();

    void Serialize(Archive& p_archive, uint32_t p_version);
};

}  // namespace cave
