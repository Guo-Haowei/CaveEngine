#pragma once
#include "cave/runtime/input/IInputConsumer.h"
#include "cave/runtime/intent/IIntentHandler.h"

#include "ShortcutDesc.h"

namespace cave {

class EditorState;
class IntentDispatcher;

class ShortcutService final : public IInputConsumer,
                              public IIntentHandler {
public:
    ShortcutService(EditorState& p_editor);
    ~ShortcutService();

    void HandleIntent(Intent& p_intent) override;
    void OnEvents(const InputFrame& p_input) override;

    const auto& GetShortcuts() const { return m_shortcuts; }
    int GetPriority() const override { return 1000; }
    DebugId GetDebugId() const override { return m_debug_id; }

private:
    void InitShortcuts();

    EditorState& m_editor;
    IntentDispatcher& m_intent_dispatcher;

    std::array<ShortcutDesc, kShortcutCount> m_shortcuts;

    const DebugId m_debug_id;
};

}  // namespace cave
