// =============================================================================
// File: cave/runtime/ecs/components/SpriteRendererComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Box.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class SpriteRendererComponent {
    CAVE_COMPONENT(SpriteRendererComponent)

private:
    CAVE_PROP(editor = Asset)
    Guid m_image_id;

    CAVE_PROP(editor = Color)
    math::Vec4f m_tint_color = math::Vec4f::One;

    CAVE_PROP()
    math::Box2 m_rect = { { 0.0f, 1.0f }, { 1.0f, 0.0f } };

    CAVE_PROP(editor = Toggle)
    bool m_flip_x = false;

    CAVE_PROP(editor = Toggle)
    bool m_flip_y = false;

    CAVE_PROP(editor = InputInt)
    int m_z_index = 0;  // higher draws later / on top

    // Non serialized
    Handle<ImageAsset> m_image_handle;

public:
    bool SetResourceGuid(const Guid& guid);
    const Guid& imageGuid() const { return m_image_id; }

    const Handle<ImageAsset> handle() const { return m_image_handle; }

    void setTintColor(const math::Vec4f& color) { m_tint_color = color; }
    const math::Vec4f& tintColor() const { return m_tint_color; }

    void setRect(const math::Box2& rect) { m_rect = rect; }
    const math::Box2& rect() const { return m_rect; }

    bool flipX() const { return m_flip_x; }
    bool flipY() const { return m_flip_y; }
    int zIndex() const { return m_z_index; }

    void OnDeserialized();
};

}  // namespace cave
