// =============================================================================
// File: cave/runtime/ui/ResolvedUI.h
// =============================================================================
#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/core/Option.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/string/StringId.h"
#include "cave/runtime/ui/UITypes.h"

namespace cave {

struct ResolvedUIElement {
    ecs::Entity entity;
    UIRect rect;
    uint32_t draw_order;
};

struct ResolvedUICanvas {
    ecs::Entity canvas;
    math::Vec2f canvas_size;

    // Parent-before-children, matching draw order.
    Vector<ResolvedUIElement> elements;
};

}  // namespace cave
