#include "ShortcutService.h"

#include "cave/core/string/StringUtils.h"
#include "cave/runtime/input/KeyState.h"
#include "cave/runtime/intent/IntentDispatcher.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/framework/IInputService.h"

#include "engine/private/core/diagnostics/DebugIdAllocator.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

#include "editor/services/EditService.h"
#include "editor/services/Workspace.h"

#include "editor/EditorIntent.h"
#include "editor/EditorState.h"

namespace cave {

ShortcutService::ShortcutService(EditorState& p_editor)
    : m_editor(p_editor)
    , m_intent_dispatcher(*p_editor.GetApp().GetIntentDispatcher())
    , m_debug_id(MakeDebugId(this)) {

    m_editor.GetApp().InputService().Register(this);
    m_intent_dispatcher.AddHandler<SaveIntent>(this);
    m_intent_dispatcher.AddHandler<UndoIntent>(this);
    m_intent_dispatcher.AddHandler<RedoIntent>(this);

    InitShortcuts();
}

ShortcutService::~ShortcutService() {
    m_intent_dispatcher.RemoveHandler<SaveIntent>(this);
    m_intent_dispatcher.RemoveHandler<UndoIntent>(this);
    m_intent_dispatcher.RemoveHandler<RedoIntent>(this);
    m_editor.GetApp().InputService().Unregister(this);
}

bool ShortcutService::HandleIntent(Intent& p_intent) {
    if (auto intent = dynamic_cast<const SaveIntent*>(&p_intent)) {
        const bool save_as = intent->SaveAs();
        LOG_OK(save_as ? "Ctrl+Shift+S" : "Ctrl+S");

        // @TODO: actually save the document
        // AssetRegistry::GetSingleton().SaveAllAssets();
        // m_editor.GetEditService().BufferCommand(std::make_shared<SaveProjectCommand>(true));
        // m_editor.GetEditService().BufferCommand(std::make_shared<SaveProjectCommand>(false));
        return true;
    }

    if (auto intent = dynamic_cast<const UndoIntent*>(&p_intent)) {
        DocId active_doc = m_editor.Workspace().FocusedDoc();
        m_editor.EditService().Undo(active_doc);
        return true;
    }

    if (auto intent = dynamic_cast<const RedoIntent*>(&p_intent)) {
        DocId active_doc = m_editor.Workspace().FocusedDoc();
        m_editor.EditService().Redo(active_doc);
        return true;
    }

    return false;
}

void ShortcutService::OnEvents(const InputFrame& p_input) {
    IInputService& input = m_editor.GetApp().InputService();
    const bool ctrl = input.GetKeyState().AnyCtrlDown();
    const bool alt = input.GetKeyState().AnyAltDown();
    const bool shift = input.GetKeyState().AnyShiftDown();

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
    m_shortcuts[std::to_underlying(Shortcut::SaveAs)] = {
        "Save As..",
        "Ctrl+Shift+S",
        [this]() {
            m_intent_dispatcher.PushIntent<SaveIntent>(true);
        },
    };
    m_shortcuts[std::to_underlying(Shortcut::Save)] = {
        "Save",
        "Ctrl+S",
        [this]() {
            m_intent_dispatcher.PushIntent<SaveIntent>(false);
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

    auto active_document = [this]() -> DocId {
        return m_editor.Workspace().FocusedDoc();
    };

    m_shortcuts[std::to_underlying(Shortcut::Redo)] = {
        "Redo",
        "Ctrl+Shift+Z",
        [active_document, this]() {
            m_intent_dispatcher.PushIntent<RedoIntent>();
        },
        [active_document, this]() { return m_editor.EditService().CanRedo(active_document()); },
    };

    m_shortcuts[std::to_underlying(Shortcut::Undo)] = {
        "Undo",
        "Ctrl+Z",
        [active_document, this]() {
            m_intent_dispatcher.PushIntent<UndoIntent>();
        },
        [active_document, this]() { return m_editor.EditService().CanUndo(active_document()); },
    };

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
