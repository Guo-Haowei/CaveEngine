#include "ShortcutService.h"

#include "cave/runtime/input/KeyCode.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/string/StringUtils.h"

#include "editor/services/EditService.h"

#include "editor/document/Document.h"
#include "editor/EditorState.h"
#include "editor/viewer/Viewer.h"

namespace cave {

ShortcutService::ShortcutService(EditorState& p_editor)
    : m_editor(p_editor) {
    InputRouter& router = m_editor.GetApp().GetInputSystem()->Router();
    router.Register(this);

    InitShortcuts();
}

ShortcutService::~ShortcutService() {
    InputRouter& router = m_editor.GetApp().GetInputSystem()->Router();
    router.Unregister(this);
}

void ShortcutService::InitShortcuts() {

    m_shortcuts[std::to_underlying(Shortcut::SaveAs)] = {
        "Save As..",
        "Ctrl+Shift+S",
        [this]() {
            LOG_WARN("Ctrl+Shift+S");
            // m_editor.GetEditService().BufferCommand(std::make_shared<SaveProjectCommand>(true));
        },
    };
    m_shortcuts[std::to_underlying(Shortcut::Save)] = {
        "Save",
        "Ctrl+S",
        [this]() {
            LOG_WARN("Ctrl+S");
            // AssetRegistry::GetSingleton().SaveAllAssets();
            // m_editor.GetEditService().BufferCommand(std::make_shared<SaveProjectCommand>(false));
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

    auto active_document = [this]() -> OldDocument* {
        if (auto tab = m_editor.GetViewer().GetActiveTab(); tab) {
            return &tab->GetDocument();
        }
        return nullptr;
    };

    m_shortcuts[std::to_underlying(Shortcut::Redo)] = {
        "Redo",
        "Ctrl+Shift+Z",
        [active_document]() { auto doc = active_document(); if (doc) doc->Redo(); },
        [active_document]() { auto doc = active_document(); return doc ? doc ->CanRedo() : false; }
    };

    m_shortcuts[std::to_underlying(Shortcut::Undo)] = {
        "Undo",
        "Ctrl+Z",
        [active_document]() { auto doc = active_document(); if (doc) doc->Undo(); },
        [active_document]() { auto doc = active_document(); return doc ? doc ->CanUndo() : false; }
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

void ShortcutService::OnEvents(const std::vector<InputEvent>& p_events) {
    InputSystem* input = m_editor.GetApp().GetInputSystem();
    const bool ctrl = input->GetKeyState().AnyCtrlDown();
    const bool alt = input->GetKeyState().AnyAltDown();
    const bool shift = input->GetKeyState().AnyShiftDown();

    for (const InputEvent& e : p_events) {
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

}  // namespace cave
