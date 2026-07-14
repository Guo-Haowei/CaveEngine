// =============================================================================
// File: cave/runtime/ui/UIComponents.h
// =============================================================================
#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/core/math/Vec.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

enum class UICanvasSpace : uint8_t {
    Screen = 0,
    World,
    Count,
};

DECLARE_ENUM_TRAITS(UICanvasSpace, "screen", "world");

struct UICanvasComponent {
    CAVE_COMPONENT(UICanvasComponent)

    CAVE_PROP(editor = EnumDropDown)
    UICanvasSpace space = UICanvasSpace::Screen;

    CAVE_PROP(editor = Translation2D)
    math::Vec2f resolution = { 1920.0f, 1080.0f };

    CAVE_PROP(editor = InputFloat)
    float match = 0.5f;  // 0 = match width, 1 = match height.
};

struct UIRectTransformComponent {
    CAVE_COMPONENT(UIRectTransformComponent)

    CAVE_PROP(editor = Translation2D)
    math::Vec2f anchor_min = math::Vec2f::Zero;

    CAVE_PROP(editor = Translation2D)
    math::Vec2f anchor_max = math::Vec2f::Zero;

    CAVE_PROP(editor = Translation2D)
    math::Vec2f offset_min = math::Vec2f::Zero;

    CAVE_PROP(editor = Translation2D)
    math::Vec2f offset_max = { 100.0f, 100.0f };
};

struct UIButtonComponent {
    CAVE_COMPONENT(UIButtonComponent)

    CAVE_PROP(editor = InputText)
    String clicked_event;

    bool interactable = true;
};

class UIImageComponent {
    CAVE_COMPONENT(UIImageComponent)

private:
    CAVE_PROP(editor = Asset)
    Guid m_image_guid;

    CAVE_PROP(editor = Color)
    math::Vec4f m_tint = math::Vec4f::One;

    // Non-serialized
    mutable Handle<ImageAsset> m_image_handle;

public:
    const Guid& imageGuid() const { return m_image_guid; }

    const math::Vec4f tint() const { return m_tint; }

    // @TODO: properly set m_image_handle
    void onDeserialized() const;

    const Handle<ImageAsset>& handle() const { return m_image_handle; }
};

struct UITextComponent {
    CAVE_COMPONENT(UITextComponent)

    String text = "Text";

    math::Vec4f tint = math::Vec4f::One;

    float font_size = 24.0f;

    // Non-serialized
    // AssetHandle font;
};

}  // namespace cave
