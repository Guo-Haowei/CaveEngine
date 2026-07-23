#include "ShortcutService.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/string/StringUtils.h"
#include "cave/runtime/input/KeyState.h"
#include "cave/runtime/intent/IntentBus.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/input/InputService.h"

#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/Workspace.h"
#include "editor/EditorIntent.h"
#include "editor/EditorState.h"

namespace cave {

ShortcutService::ShortcutService(EditorState& editor)
    : m_editor(editor)
    , m_engine_services(editor.app().services())
    , m_editor_services(editor.services())
    , m_debug_id(MakeDebugId(this)) {

    m_engine_services.inputService().addConsumer(this);
    m_engine_services.intentBus().addHandler<SaveIntent>(this);
    m_engine_services.intentBus().addHandler<SaveAllIntent>(this);
    m_engine_services.intentBus().addHandler<UndoIntent>(this);
    m_engine_services.intentBus().addHandler<RedoIntent>(this);

    initShortcuts();
}

ShortcutService::~ShortcutService() {
    m_engine_services.intentBus().removeHandler<SaveIntent>(this);
    m_engine_services.intentBus().removeHandler<SaveAllIntent>(this);
    m_engine_services.intentBus().removeHandler<UndoIntent>(this);
    m_engine_services.intentBus().removeHandler<RedoIntent>(this);
    m_engine_services.inputService().removeConsumer(this);
}

bool ShortcutService::handleIntent(Intent& intent) {
    if (auto save = dynamic_cast<const SaveIntent*>(&intent)) {
        m_editor_services.document().save(save->doc_id());
        return true;
    }

    if (auto save_all = dynamic_cast<const SaveAllIntent*>(&intent)) {
        m_editor_services.document().saveAll();
        return true;
    }

    if (auto undo = dynamic_cast<const UndoIntent*>(&intent)) {
        m_editor_services.edit().undo(undo->doc_id());
        return true;
    }

    if (auto redo = dynamic_cast<const RedoIntent*>(&intent)) {
        m_editor_services.edit().redo(redo->doc_id());
        return true;
    }

    return false;
}

void ShortcutService::onEvents(const InputFrame& input) {
    const auto& key_state = m_engine_services.inputService().keyState();
    const bool ctrl = key_state.anyCtrlDown();
    const bool alt = key_state.anyAltDown();
    const bool shift = key_state.anyShiftDown();

    for (const InputEvent& e : input.events) {
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

void ShortcutService::initShortcuts() {
    auto active_document = [this]() -> DocId {
        return m_editor_services.workspace().focusedDoc();
    };

    m_shortcuts[std::to_underlying(Shortcut::SaveAs)] = {
        "Save As..",
        "Ctrl+Shift+S",
        [this]() {
            m_engine_services.intentBus().queue<SaveAllIntent>();
        },
    };
    m_shortcuts[std::to_underlying(Shortcut::Save)] = {
        "Save",
        "Ctrl+S",
        [active_document, this]() {
            m_engine_services.intentBus().queue<SaveIntent>(active_document());
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
            if (m_editor_services.edit().canRedo(active_document()))
                m_engine_services.intentBus().queue<RedoIntent>(active_document());
        },
        [active_document, this]() { return m_editor_services.edit().canRedo(active_document()); },
    };

    m_shortcuts[std::to_underlying(Shortcut::Undo)] = {
        "Undo",
        "Ctrl+Z",
        [active_document, this]() {
            if (m_editor_services.edit().canUndo(active_document()))
                m_engine_services.intentBus().queue<UndoIntent>(active_document());
        },
        [active_document, this]() { return m_editor_services.edit().canUndo(active_document()); },
    };

    // @TODO: make this an intent
    m_shortcuts[std::to_underlying(Shortcut::Debug)] = {
        "Start Debugging",
        "F5",
        // @TODO: move requestModeSwitch to PIE
        [this]() { m_editor.requestModeSwitch(); },
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
        while (split.canAdvance()) {
            std::string_view sv = split.advance('+');
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
