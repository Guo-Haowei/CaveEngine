#pragma once
#include "cave/runtime/input/IInputConsumer.h"
#include "cave/runtime/intent/IIntentHandler.h"

#include "ShortcutDesc.h"

namespace cave {

struct EngineServices;
struct EditorServices;

// @TODO: remove this
class EditorState;

class ShortcutService final : public IInputConsumer,
                              public IIntentHandler {
public:
    ShortcutService(EditorState& editor);
    ~ShortcutService();

    bool handleIntent(Intent& intent) override;

    const auto& getShortcuts() const { return m_shortcuts; }

    void onEvents(const InputFrame& input) override;
    int priority() const override { return 1000; }
    DebugId debugId() const override { return m_debug_id; }

private:
    void initShortcuts();

    EditorState& m_editor;
    EngineServices& m_engine_services;
    EditorServices& m_editor_services;
    const DebugId m_debug_id;

    std::array<ShortcutDesc, kShortcutCount> m_shortcuts;
};

}  // namespace cave
