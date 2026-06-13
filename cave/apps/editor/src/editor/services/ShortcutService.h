#pragma once
#include "cave/runtime/input/IInputConsumer.h"
#include "cave/runtime/intent/IIntentHandler.h"

#include "ShortcutDesc.h"

namespace cave {

struct AppServices;
struct EditorServices;

// @TODO: remove this
class EditorState;

class ShortcutService final : public IInputConsumer,
                              public IIntentHandler {
public:
    ShortcutService(EditorState& editor);
    ~ShortcutService();

    bool handleIntent(Intent& intent) override;

    const auto& getShortcuts() const { return shortcuts_; }

    void onEvents(const InputFrame& input) override;
    int priority() const override { return 1000; }
    DebugId debugId() const override { return debug_id_; }

private:
    void initShortcuts();

    EditorState& editor_;
    AppServices& app_services_;
    EditorServices& editor_services_;
    const DebugId debug_id_;

    std::array<ShortcutDesc, kShortcutCount> shortcuts_;
};

}  // namespace cave
