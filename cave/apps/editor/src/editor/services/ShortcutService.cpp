#include "ShortcutService.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/string/StringUtils.h"
#include "cave/runtime/input/KeyState.h"
#include "cave/runtime/intent/IntentDispatcher.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/input/InputService.h"

#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/Workspace.h"
#include "editor/EditorIntent.h"
#include "editor/EditorState.h"

namespace cave {

ShortcutService::ShortcutService(EditorState& editor)
    : editor_(editor)
    , app_services_(editor.app().services())
    , editor_services_(editor.services())
    , debug_id_(MakeDebugId(this)) {

    app_services_.inputService().addConsumer(this);
    app_services_.intentDispatcher().addHandler<SaveIntent>(this);
    app_services_.intentDispatcher().addHandler<UndoIntent>(this);
    app_services_.intentDispatcher().addHandler<RedoIntent>(this);

    initShortcuts();
}

ShortcutService::~ShortcutService() {
    app_services_.intentDispatcher().removeHandler<SaveIntent>(this);
    app_services_.intentDispatcher().removeHandler<UndoIntent>(this);
    app_services_.intentDispatcher().removeHandler<RedoIntent>(this);
    app_services_.inputService().removeConsumer(this);
}

bool ShortcutService::handleIntent(Intent& intent) {
    if (auto save = dynamic_cast<const SaveIntent*>(&intent)) {
        editor_services_.document().save(save->doc_id());
        return true;
    }

    if (auto undo = dynamic_cast<const UndoIntent*>(&intent)) {
        editor_services_.edit().undo(undo->doc_id());
        return true;
    }

    if (auto redo = dynamic_cast<const RedoIntent*>(&intent)) {
        editor_services_.edit().redo(redo->doc_id());
        return true;
    }

    return false;
}

void ShortcutService::onEvents(const InputFrame& input) {
    auto& key_state = app_services_.inputService().keyState();
    const bool ctrl = key_state.anyCtrlDown();
    const bool alt = key_state.anyAltDown();
    const bool shift = key_state.anyShiftDown();

    for (const InputEvent& e : input.events) {
        if (e.type != InputEventType::ButtonDown)
            continue;

        for (const ShortcutDesc& desc : shortcuts_) {
            if (static_cast<uint32_t>(desc.key) != e.code) continue;
            if (desc.ctrl)
                if (!ctrl) continue;
            if (desc.shift)
                if (!shift) continue;
            if (desc.alt)
                if (!alt) continue;

            // LOG_VERBOSE("ShortcutService::OnEvents: shortcut '{}' fired", desc.shortcut);
            desc.execute_func();
            e.consumed = true;
            break;
        }
    }
}

void ShortcutService::initShortcuts() {
    auto active_document = [this]() -> DocId {
        return editor_services_.workspace().focusedDoc();
    };

    shortcuts_[std::to_underlying(Shortcut::SaveAs)] = {
        "Save As..",
        "Ctrl+Shift+S",
        [active_document, this]() {
            app_services_.intentDispatcher().queue<SaveIntent>(active_document());
        },
    };
    shortcuts_[std::to_underlying(Shortcut::Save)] = {
        "Save",
        "Ctrl+S",
        [active_document, this]() {
            app_services_.intentDispatcher().queue<SaveIntent>(active_document());
        },
    };

    shortcuts_[std::to_underlying(Shortcut::Open)] = {
        "Open",
        "Ctrl+O",
        [this]() {
            LOG_WARN("Ctrl+O");
            // m_editor.BufferCommand(std::make_shared<OpenProjectCommand>(true));
        },
    };

    shortcuts_[std::to_underlying(Shortcut::Redo)] = {
        "Redo",
        "Ctrl+Shift+Z",
        [active_document, this]() {
            if (editor_services_.edit().canRedo(active_document()))
                app_services_.intentDispatcher().queue<RedoIntent>(active_document());
        },
        [active_document, this]() { return editor_services_.edit().canRedo(active_document()); },
    };

    shortcuts_[std::to_underlying(Shortcut::Undo)] = {
        "Undo",
        "Ctrl+Z",
        [active_document, this]() {
            if (editor_services_.edit().canUndo(active_document()))
                app_services_.intentDispatcher().queue<UndoIntent>(active_document());
        },
        [active_document, this]() { return editor_services_.edit().canUndo(active_document()); },
    };

    // @TODO: make this an intent
    shortcuts_[std::to_underlying(Shortcut::Debug)] = {
        "Start Debugging",
        "F5",
        // @TODO: move RequestModeSwitch away from editor
        [this]() { editor_.RequestModeSwitch(); },
        []() { return true; },
    };

    // @TODO: proper key mapping
    std::map<std::string_view, Key> keyMapping = {
        { "A", Key::A },
        { "B", Key::B },
        { "C", Key::C },
        { "D", Key::D },
        { "E", Key::E },
        { "F", Key::F },
        { "G", Key::G },
        { "H", Key::H },
        { "I", Key::I },
        { "J", Key::J },
        { "K", Key::K },
        { "L", Key::L },
        { "M", Key::M },
        { "N", Key::N },
        { "O", Key::O },
        { "P", Key::P },
        { "Q", Key::Q },
        { "R", Key::R },
        { "S", Key::S },
        { "T", Key::T },
        { "U", Key::U },
        { "V", Key::V },
        { "W", Key::W },
        { "X", Key::X },
        { "Y", Key::Y },
        { "Z", Key::Z },
        { "F5", Key::F5 },
    };

    // @TODO: compile time
    for (ShortcutDesc& shortcut : shortcuts_) {
        StringSplitter split(shortcut.shortcut);
        while (split.CanAdvance()) {
            std::string_view sv = split.Advance('+');
            if (sv == "Ctrl") {
                shortcut.ctrl = true;
            } else if (sv == "Shift") {
                shortcut.shift = true;
            } else if (sv == "Alt") {
                shortcut.alt = true;
            } else {
                auto it = keyMapping.find(sv);
                DEV_ASSERT(it != keyMapping.end());
                shortcut.key = it->second;
            }
        }
    }
}

}  // namespace cave
