#include "editor_state.h"

#include "engine/debugger/profiler.h"
#include "engine/runtime/application.h"
#include "engine/runtime/imgui_manager.h"

// @TODO: refactor
#include <imgui/imgui_internal.h>
#include <imnodes/imnodes.h>

#include "engine/assets/image_asset.h"
#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/input_manager.h"
#include "engine/runtime/scene_manager_interface.h"
#include "engine/runtime/script_manager.h"
#include "engine/ui/layout.h"

#include "editor/document/document.h"
#include "editor/editor_command.h"
#include "editor/editor_dvars.h"
#include "editor/panels/asset_inspector.h"
#include "editor/panels/file_system_panel.h"
#include "editor/panels/hierarchy_panel.h"
#include "editor/panels/log_panel.h"
#include "editor/panels/menu_bar.h"
#include "editor/panels/property_panel.h"
#include "editor/panels/render_graph_viewer.h"
#include "editor/panels/renderer_panel.h"
#include "editor/viewer/viewer.h"
#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/image.h"

namespace cave {

EditorState::EditorState(Application& p_app)
    : AppState(p_app) {

    m_asset_inspector = std::make_shared<AssetInspector>(*this);
    m_menu_bar = std::make_shared<MenuBar>(*this);
    m_viewer = std::make_shared<Viewer>(*this);
    m_log_panel = std::make_shared<LogPanel>(*this);
    m_file_system_panel = std::make_shared<FileSystemPanel>(*this);

    AddPanel(m_log_panel);
    AddPanel(std::make_shared<RendererPanel>(*this));
    AddPanel(std::make_shared<HierarchyPanel>(*this));
    AddPanel(std::make_shared<PropertyPanel>(*this));
    AddPanel(m_viewer);
    AddPanel(m_asset_inspector);
    AddPanel(std::make_shared<RenderGraphViewer>(*this));
    AddPanel(m_file_system_panel);

    // @TODO: refactor this at some point
    m_shortcuts[SHORT_CUT_SAVE_AS] = {
        "Save As..",
        "Ctrl+Shift+S",
        [this]() {
            BufferCommand(std::make_shared<SaveProjectCommand>(true));
        },
    };
    m_shortcuts[SHORT_CUT_SAVE] = {
        "Save",
        "Ctrl+S",
        [&]() {
            AssetRegistry::GetSingleton().SaveAllAssets();
            // this->BufferCommand(std::make_shared<SaveProjectCommand>(false));
        },
    };
    m_shortcuts[SHORT_CUT_OPEN] = {
        "Open",
        "Ctrl+O",
        //[&]() { this->BufferCommand(std::make_shared<OpenProjectCommand>(true)); },
    };

    auto active_document = [this]() -> Document* {
        if (auto tab = m_viewer->GetActiveTab(); tab) {
            return &tab->GetDocument();
        }
        return nullptr;
    };

    m_shortcuts[SHORT_CUT_REDO] = {
        "Redo",
        "Ctrl+Shift+Z",
        [active_document]() { auto doc = active_document(); if (doc) doc->Redo(); },
        [active_document]() { auto doc = active_document(); return doc ? doc ->CanRedo() : false; }
    };

    m_shortcuts[SHORT_CUT_UNDO] = {
        "Undo",
        "Ctrl+Z",
        [active_document]() { auto doc = active_document(); if (doc) doc->Undo(); },
        [active_document]() { auto doc = active_document(); return doc ? doc ->CanUndo() : false; }
    };

    m_shortcuts[SHORT_CUT_DEBUG] = {
        "Start Debugging",
        "F5",
        [this]() { RequestGamePlay(); },
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

    for (auto& shortcut : m_shortcuts) {
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
                if (sv.length() == 1) {
                    auto it = keyMapping.find(sv);
                    if (it == keyMapping.end()) {
                        CRASH_NOW();
                    }
                    shortcut.key = it->second;
                }
            }
        }
    }
}

void EditorState::OnEnter(const StateRequest& p_args) {
    unused(p_args);

    ImNodes::CreateContext();

    auto handle = AssetRegistry::GetSingleton().FindByPath<ImageAsset>("@persist://textures/checkerboard");
    if (handle.is_some()) {
        context.checkerboard = handle.unwrap_unchecked().Wait();
    }

    // m_app.GetInputManager()->PushInputHandler(this);

    for (auto& panel : m_panels) {
        panel->OnAttach();
    }

    if (auto asset = DVAR_GET_STRING(last_open_asset); !asset.empty()) {
        if (auto res = Guid::Parse(asset); res.is_some()) {
            Guid guid = res.unwrap_unchecked();
            CommandInspectAsset(guid);
        }
    }
}

void EditorState::OnExit() {
    //[[maybe_unused]] auto handler = m_app.GetInputManager()->PopInputHandler();
    // DEV_ASSERT(handler == this);

    ImNodes::DestroyContext();
}

void EditorState::Tick(float p_timestep) {
    CAVE_PROFILE_EVENT();
    context.timestep = p_timestep;

    ImguiManager* imgui_manager = m_app.GetImguiManager();
    DEV_ASSERT(imgui_manager);

    // @TODO: refactor this
    imgui_manager->BeginFrame();

    // @TODO: DO NOT Request SCENE here
    Scene* scene = m_app.GetSceneManager()->GetActiveScene().get();

    FlushInputEvents();

    DockSpace();
    for (auto& it : m_panels) {
        it->Update();
    }

    // @TODO: fix this as well
    FlushCommand(scene);

    CAVE_PROFILE_EVENT("ImGui::Render");
    ImGui::Render();
}

Option<StateRequest> EditorState::PopRequest() {
    auto request = m_request;
    m_request = None();
    return request;
}

void EditorState::RequestGamePlay() {
    m_request = Some(StateRequest{ AppStateId::Runtime });
}

void EditorState::AddPanel(std::shared_ptr<EditorItem> p_panel) {
    m_panels.emplace_back(p_panel);
}

void EditorState::DockSpace() {
    CAVE_PROFILE_EVENT();

    ui::DockSpace({
        "DockSpace Demo",
        [this]() { m_menu_bar->Update(); },
        [this]() {
            CompositeLogger& logger = CompositeLogger::GetSingleton();
            const uint32_t error_count = static_cast<uint32_t>(logger.GetErrorLogs().size());
            const uint32_t warning_count = static_cast<uint32_t>(logger.GetWarningLogs().size());

            ui::ErrorIcon();

            ImGui::SameLine();
            ImGui::Text(" %u Error(s)", error_count);

            ImGui::SameLine();
            ui::WarningIcon();

            ImGui::SameLine();
            ImGui::Text(" %u Warning(s)", warning_count);
        },
    });

    return;
}

////////////////////
////////////////////

void EditorState::FlushInputEvents() {
    CAVE_PROFILE_EVENT();

#if 0
    for (auto& event : m_buffered_events) {
        if (m_viewer->IsFocused() || m_viewer->IsHovered()) {
            if (m_viewer->HandleInput(event.get())) {
                continue;
            }
        }

        if (auto e = dynamic_cast<InputEventKey*>(event.get()); e) {
            for (auto shortcut : m_shortcuts) {
                // @TODO: refactor this
                auto is_key_handled = [&]() {
                    if (!e->IsPressed()) {
                        return false;
                    }
                    if (e->GetKey() != shortcut.key) {
                        return false;
                    }
                    if (e->IsAltPressed() != shortcut.alt) {
                        return false;
                    }
                    if (e->IsShiftPressed() != shortcut.shift) {
                        return false;
                    }
                    if (e->IsCtrlPressed() != shortcut.ctrl) {
                        return false;
                    }
                    return true;
                };
                if (is_key_handled()) {
                    if (shortcut.executeFunc) {
                        shortcut.executeFunc();
                    }

                    break;
                }
            }
        }
    }
#endif

    m_buffered_events.clear();
}

// @TODO: these are associated with scene editor, move to scene editor
void EditorState::BufferCommand(std::shared_ptr<EditorCommandBase>&& p_command) {
    p_command->m_editor = this;
    m_command_buffer.emplace_back(std::move(p_command));
}

void EditorState::CommandInspectAsset(const Guid& p_guid) {
    auto command = std::make_shared<EditorInspectAssetCommand>(p_guid);
    BufferCommand(command);
}

void EditorState::CommandAddComponent(ComponentName p_type, ecs::Entity p_target) {
    auto command = std::make_shared<EditorCommandAddComponent>(p_type);
    command->target = p_target;
    BufferCommand(command);
}

void EditorState::CommandAddEntity(EntityType p_type, ecs::Entity p_parent) {
    auto command = std::make_shared<EditorCommandAddEntity>(p_type);
    command->m_parent = p_parent;
    BufferCommand(command);
}

void EditorState::CommandRemoveEntity(ecs::Entity p_target) {
    auto command = std::make_shared<EditorCommandRemoveEntity>(p_target);
    BufferCommand(command);
}

void EditorState::CommandDuplicateEntity(ecs::Entity p_target) {
    auto command = std::make_shared<EditorCommandDuplicateEntity>(p_target);
    BufferCommand(command);
}

void EditorState::FlushCommand(Scene* p_scene) {
    CAVE_PROFILE_EVENT();

    while (!m_command_buffer.empty()) {
        auto task = m_command_buffer.front();
        m_command_buffer.pop_front();
        task->Execute(*p_scene);
    }
}

}  // namespace cave
