#pragma once
#include "cave/runtime/input/IInputConsumer.h"

#include "ShortcutDesc.h"

namespace cave {

class EditorState;

class ShortcutService : public IInputConsumer {
public:
    ShortcutService(EditorState& p_editor);
    ~ShortcutService();

    int GetPriority() const override { return 1000; }
    void OnEvents(const std::vector<InputEvent>& p_events) override;

    const auto& GetShortcuts() const { return m_shortcuts; }

private:
    void InitShortcuts();

    EditorState& m_editor;

    std::array<ShortcutDesc, kShortcutCount> m_shortcuts;
};

}  // namespace cave
