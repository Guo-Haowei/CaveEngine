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
    Guid image_id_;

    CAVE_PROP(editor = Color)
    math::Vec4f tint_color_ = math::Vec4f::One;

    CAVE_PROP()
    math::Box2 rect_ = { { 0.0f, 1.0f }, { 1.0f, 0.0f } };

    CAVE_PROP(editor = Toggle)
    bool flip_x_ = false;

    CAVE_PROP(editor = Toggle)
    bool flip_y_ = false;

    CAVE_PROP(editor = InputInt)
    int z_index_ = 0;  // higher draws later / on top

    // Non serialized
    Handle<ImageAsset> image_handle_;

public:
    bool SetResourceGuid(const Guid& guid);
    const Guid& imageGuid() const { return image_id_; }

    const Handle<ImageAsset> handle() const { return image_handle_; }

    void setTintColor(const math::Vec4f& color) { tint_color_ = color; }
    const math::Vec4f& tintColor() const { return tint_color_; }

    void setRect(const math::Box2& rect) { rect_ = rect; }
    const math::Box2& rect() const { return rect_; }

    bool flipX() const { return flip_x_; }
    bool flipY() const { return flip_y_; }
    int zIndex() const { return z_index_; }

    void OnDeserialized();
};

}  // namespace cave
