#pragma once
#include "engine/reflection/reflection.h"
#include "engine/assets/asset_handle.h"
#include "engine/math/box.h"
#include "engine/math/geomath.h"

namespace cave {

class SpriteRendererComponent {
    CAVE_META(SpriteRendererComponent)

private:
    CAVE_PROP(editor = Asset)
    Guid m_image_id;

    CAVE_PROP(editor = Color)
    Vector4f m_tint_color = Vector4f::One;

    CAVE_PROP()
    Rect m_rect = { Vector2f::Zero, Vector2f::One };

    CAVE_PROP(editor = Toggle)
    bool m_flip_x = false;

    CAVE_PROP(editor = Toggle)
    bool m_flip_y = false;

    CAVE_PROP(editor = Toggle)
    bool m_is_billboard = false;

    // Non serialized
    Handle<ImageAsset> m_image_handle;

public:
    bool SetResourceGuid(const Guid& p_guid);
    const Guid& GetResourceGuid() const { return m_image_id; }

    const Handle<ImageAsset> GetHandle() const { return m_image_handle; }

    void SetTintColor(const Vector4f& p_tint_color) { m_tint_color = p_tint_color; }
    const Vector4f& GetTintColor() const { return m_tint_color; }

    void SetRect(const Rect& p_rect) { m_rect = p_rect; }
    const Rect& GetRect() const { return m_rect; }

    void OnDeserialized();
};

}  // namespace cave
