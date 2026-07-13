// =============================================================================
// File: cave/ui/UIDrawCommand.h
// =============================================================================
#pragma once
#include "cave/core/Color.h"
#include "cave/core/containers/Containers.h"
#include "cave/runtime/ui/UITypes.h"

namespace cave {

enum class UIDrawCommandType {
    Rect,
};

struct UIDrawRectCommand {
    OldUIRect rect;
    Color color;
};

struct UIDrawCommand {
    UIDrawCommandType type = UIDrawCommandType::Rect;

    union {
        UIDrawRectCommand rect;
    };
};

struct UIDrawList {
    Vector<UIDrawCommand> cmds;

    void addRect(const OldUIRect& rect, const Color& color) {
        UIDrawCommand cmd{
            .type = UIDrawCommandType::Rect,
            .rect = {
                .rect = rect,
                .color = color,
            },
        };
        cmds.push_back(cmd);
    }
};

}  // namespace cave
