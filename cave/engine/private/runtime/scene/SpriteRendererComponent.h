#pragma once
#include "engine/private/reflection/reflection.h"
#include "engine/private/assets/asset_handle.h"
#include "engine/private/math/box.h"
#include "engine/private/math/geomath.h"

namespace cave {

class SpriteRendererComponent {
    CAVE_META(SpriteRendererComponent)

private:
    CAVE_PROP(editor = Asset)
    Guid m_image_id;

    CAVE_PROP(editor = Color)
    math::Vector4f m_tint_color = math::Vector4f::One;

    CAVE_PROP()
    math::Box2 m_rect = { math::Vector2f::Zero, math::Vector2f::One };

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

    void SetTintColor(const math::Vector4f& p_tint_color) { m_tint_color = p_tint_color; }
    const math::Vector4f& GetTintColor() const { return m_tint_color; }

    void SetRect(const math::Box2& p_rect) { m_rect = p_rect; }
    const math::Box2& GetRect() const { return m_rect; }

    void OnDeserialized();
};

}  // namespace cave
