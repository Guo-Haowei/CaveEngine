// =============================================================================
// File: cave/render/components/BackgroundRendererComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vec.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct FieldChange;

class BackgroundRendererComponent {
    CAVE_COMPONENT(BackgroundRendererComponent)

private:
    CAVE_PROP(editor = Asset, on_change = onImageGuidChanged)
    Guid m_image_guid;

    CAVE_PROP(editor = Translation2D)
    math::Vec2f m_repeat_size = { 1.0f, 1.0f };

    CAVE_PROP(editor = Translation2D)
    math::Vec2f m_parallax = { 1.0f, 1.0f };

    CAVE_PROP(editor = Color)
    math::Vec4f m_tint = math::Vec4f::One;

    // Non serialized
    Handle<ImageAsset> m_image_handle;

    void refreshImageHandle();
    void onImageGuidChanged(const FieldChange& change);

public:
    math::Vec2f repeatSize() const { return m_repeat_size; }
    void setRepeatSize(math::Vec2f value) { m_repeat_size = value; }

    math::Vec2f parallax() const { return m_parallax; }
    void setParallax(math::Vec2f value) { m_parallax = value; }

    const math::Vec4f& tint() const { return m_tint; }
    void setTint(const math::Vec4f& tint) { m_tint = tint; }

    const Handle<ImageAsset>& handle() const { return m_image_handle; }

    void onDeserialized();
};

}  // namespace cave
