#pragma once
#include "editor/IEditorItem.h"

namespace cave {

struct WindowState {
    bool open{ false };
    bool visible{ false };
    bool hovered{ false };
    bool focused{ false };
};

class EditorWindow : public IEditorItem {
public:
    EditorWindow(EditorState& editor);

    void drawUI() override;

    virtual const char* windowId() const = 0;

    bool isOpen() const { return state_.open; }
    bool isVisible() const { return state_.visible; }
    bool isFocused() const { return state_.focused; }
    bool isHovered() const { return state_.hovered; }

protected:
    virtual void drawUIImpl() = 0;
    void resetState();
    void updateState();

    WindowState state_;

    int flags_{ 0 };
};

}  // namespace cave
