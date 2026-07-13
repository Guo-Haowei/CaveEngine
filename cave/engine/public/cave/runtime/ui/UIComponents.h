// =============================================================================
// File: cave/runtime/ui/UIComponents.h
// =============================================================================
#pragma once
#include <cstdint>

#include "cave/core/math/Vec.h"
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
    math::Vec2f reference_resolution = { 1920.0f, 1080.0f };

    CAVE_PROP(editor = InputFloat)
    float match_width_or_height = 0.5f;  // 0 = match width, 1 = match height.
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

    CAVE_PROP(editor = StringId)
    StringId clicked_event;

    bool interactable = true;
};

}  // namespace cave
