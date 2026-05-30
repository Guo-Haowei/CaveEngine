// =============================================================================
// File: engine/public/cave/ui/UIDrawCommand.h
// =============================================================================
#pragma once
#include <vector>
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
    std::vector<UIDrawCommand> cmds;

    void AddRect(const UIRect& p_rect, const UIColor& p_color) {
        UIDrawCommand cmd{
            .type = UIDrawCommandType::Rect,
            .rect = {
                .rect = p_rect,
                .color = p_color,
            },
        };
        cmds.push_back(cmd);
    }
};

}  // namespace cave
