#include "ShortcutService.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/string/StringUtils.h"
#include "cave/runtime/input/KeyState.h"
#include "cave/runtime/intent/IntentDispatcher.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/input/InputService.h"

#include "editor/services/EditService.h"
#include "editor/services/Workspace.h"
#include "editor/EditorIntent.h"
#include "editor/EditorState.h"

namespace cave {

ShortcutService::ShortcutService(EditorState& p_editor)
    : m_editor(p_editor)
    , input_service_(p_editor.app().services().inputService())
    , intent_dispatcher_(p_editor.app().services().intentDispatcher())
    , m_debug_id(MakeDebugId(this)) {

    input_service_.addConsumer(this);
    intent_dispatcher_.addHandler<SaveIntent>(this);
    intent_dispatcher_.addHandler<UndoIntent>(this);
    intent_dispatcher_.addHandler<RedoIntent>(this);

    InitShortcuts();
}

ShortcutService::~ShortcutService() {
    intent_dispatcher_.removeHandler<SaveIntent>(this);
    intent_dispatcher_.removeHandler<UndoIntent>(this);
    intent_dispatcher_.removeHandler<RedoIntent>(this);
    input_service_.removeConsumer(this);
}

bool ShortcutService::handleIntent(Intent& p_intent) {
    if (auto intent = dynamic_cast<const SaveIntent*>(&p_intent)) {
        const bool save_as = intent->save_as;
        LOG_OK(save_as ? "Ctrl+Shift+S" : "Ctrl+S");

        // @TODO: actually save the document
        // AssetRegistry::GetSingleton().SaveAllAssets();
        // m_editor.GetEditService().BufferCommand(std::make_shared<SaveProjectCommand>(true));
        // m_editor.GetEditService().BufferCommand(std::make_shared<SaveProjectCommand>(false));
        return true;
    }

    if (auto intent = dynamic_cast<const UndoIntent*>(&p_intent)) {
        m_editor.EditService().Undo(intent->doc_id);
        return true;
    }

    if (auto intent = dynamic_cast<const RedoIntent*>(&p_intent)) {
        m_editor.EditService().Redo(intent->doc_id);
        return true;
    }

    return false;
}

void ShortcutService::onEvents(const InputFrame& p_input) {
    const bool ctrl = input_service_.keyState().anyCtrlDown();
    const bool alt = input_service_.keyState().anyAltDown();
    const bool shift = input_service_.keyState().anyShiftDown();

    for (const InputEvent& e : p_input.events) {
        if (e.type != InputEventType::ButtonDown)
            continue;

        for (const ShortcutDesc& desc : m_shortcuts) {
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

void ShortcutService::InitShortcuts() {
    auto active_document = [this]() -> DocId {
        return m_editor.Workspace().focusedDoc();
    };

    m_shortcuts[std::to_underlying(Shortcut::SaveAs)] = {
        "Save As..",
        "Ctrl+Shift+S",
        [active_document, this]() {
            intent_dispatcher_.queue<SaveIntent>(active_document(), true);
        },
    };
    m_shortcuts[std::to_underlying(Shortcut::Save)] = {
        "Save",
        "Ctrl+S",
        [active_document, this]() {
            intent_dispatcher_.queue<SaveIntent>(active_document(), false);
        },
    };

    m_shortcuts[std::to_underlying(Shortcut::Open)] = {
        "Open",
        "Ctrl+O",
        [this]() {
            LOG_WARN("Ctrl+O");
            // m_editor.BufferCommand(std::make_shared<OpenProjectCommand>(true));
        },
    };

    m_shortcuts[std::to_underlying(Shortcut::Redo)] = {
        "Redo",
        "Ctrl+Shift+Z",
        [active_document, this]() {
            if (m_editor.EditService().CanRedo(active_document()))
                intent_dispatcher_.queue<RedoIntent>(active_document());
        },
        [active_document, this]() { return m_editor.EditService().CanRedo(active_document()); },
    };

    m_shortcuts[std::to_underlying(Shortcut::Undo)] = {
        "Undo",
        "Ctrl+Z",
        [active_document, this]() {
            if (m_editor.EditService().CanUndo(active_document()))
                intent_dispatcher_.queue<UndoIntent>(active_document());
        },
        [active_document, this]() { return m_editor.EditService().CanUndo(active_document()); },
    };

    // @TODO: make this an intent
    m_shortcuts[std::to_underlying(Shortcut::Debug)] = {
        "Start Debugging",
        "F5",
        [this]() { m_editor.RequestModeSwitch(); },
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
    for (ShortcutDesc& shortcut : m_shortcuts) {
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
