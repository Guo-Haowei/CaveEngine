#pragma once
#include "cave/runtime/input/IInputConsumer.h"
#include "cave/runtime/input/KeyCode.h"

namespace cave {

class EditorState;

enum class Shortcut : uint8_t {
    Save = 0,
    SaveAs,
    Open,
    Undo,
    Redo,
    Debug,

    _Count,
};

struct ShortcutDesc {
    const char* name{ nullptr };
    const char* shortcut{ nullptr };
    std::function<void()> execute_func{ nullptr };
    std::function<bool()> enabled_func{ nullptr };

    Key key{};
    bool ctrl{};
    bool alt{};
    bool shift{};
};

class ShortcutManager : public IInputConsumer {
public:
    ShortcutManager(EditorState& p_editor);
    ~ShortcutManager();

    int GetPriority() const override { return 1000; }
    void OnEvents(const std::vector<InputEvent>& p_events) override;

    const auto& GetShortcuts() const { return m_shortcuts; }

private:
    void InitShortcuts();

    EditorState& m_editor;

    std::array<ShortcutDesc, std::to_underlying(Shortcut::_Count)> m_shortcuts;
};

}  // namespace cave
