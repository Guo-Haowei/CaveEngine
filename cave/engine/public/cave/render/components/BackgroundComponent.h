// =============================================================================
// File: cave/render/components/BackgroundComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vec.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct FieldChange;

struct BackgroundComponent {
    CAVE_COMPONENT(BackgroundComponent)

    CAVE_PROP(editor = Translation2D)
    math::Vec2f repeat_size = { 1.0f, 1.0f };

    CAVE_PROP(editor = Translation2D)
    math::Vec2f parallax = { 1.0f, 1.0f };
};

}  // namespace cave
