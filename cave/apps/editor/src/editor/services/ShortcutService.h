#pragma once
#include "cave/runtime/input/IInputConsumer.h"

#include "ShortcutDesc.h"

namespace cave {

class EditorState;

class ShortcutService final : public IInputConsumer {
public:
    ShortcutService(EditorState& p_editor);
    ~ShortcutService();

    int GetPriority() const override { return 1000; }
    void OnEvents(const InputFrame& p_input) override;

    const auto& GetShortcuts() const { return m_shortcuts; }

    DebugId GetDebugId() override { return m_debug_id; }

private:
    void InitShortcuts();

    EditorState& m_editor;
    const DebugId m_debug_id;

    std::array<ShortcutDesc, kShortcutCount> m_shortcuts;
};

}  // namespace cave
