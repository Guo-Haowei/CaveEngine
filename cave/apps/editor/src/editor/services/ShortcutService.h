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
    ShortcutService(EditorState& editor);
    ~ShortcutService();

    bool HandleIntent(Intent& intent) override;

    const auto& getShortcuts() const { return m_shortcuts; }

    void onEvents(const InputFrame& input) override;
    int priority() const override { return 1000; }
    DebugId debugId() const override { return m_debug_id; }

private:
    void InitShortcuts();

    EditorState& m_editor;
    IntentDispatcher& m_intent_dispatcher;

    std::array<ShortcutDesc, kShortcutCount> m_shortcuts;

    const DebugId m_debug_id;
};

}  // namespace cave
