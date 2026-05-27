// =============================================================================
// File: engine/public/cave/ui/UIDrawCommand.h
// =============================================================================
#pragma once
#include "cave/ui/UITypes.h"

namespace cave {

enum class UIDrawCommandType {
    Rect,
};

struct UIDrawRectCommand {
    UIRect rect;
    UIColor color;
};

struct UIDrawCommand {
    UIDrawCommandType type = UIDrawCommandType::Rect;

    union {
        UIDrawRectCommand rect;
    };
};

struct UIDrawList {
    std::vector<UIDrawRectCommand> rects;

    void Clear() {
        rects.clear();
    }

    void AddRect(const UIRect& p_rect, const UIColor& p_color) {
        rects.push_back({ p_rect, p_color });
    }
};

}  // namespace cave
