#include "editor_layer.h"

#include <imgui/imgui_internal.h>
#include <imnodes/imnodes.h>

#include "engine/assets/image_asset.h"
#include "engine/core/string/string_utils.h"
#include "engine/input/input_event.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/input_manager.h"
#include "engine/runtime/layer.h"
#include "engine/runtime/scene_manager_interface.h"
#include "engine/runtime/script_manager.h"

#include "editor/document/document.h"
#include "editor/editor_command.h"
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
#include "editor/widgets/widget.h"

// @NOTE: include dvars at last
#include "engine/renderer/graphics_dvars.h"

namespace cave {

EditorLayer::EditorLayer()
    : Layer("EditorLayer") {

    m_menu_bar = std::make_shared<MenuBar>(*this);
    m_viewer = std::make_shared<Viewer>(*this);
    m_log_panel = std::make_shared<LogPanel>(*this);

    AddPanel(m_log_panel);
    AddPanel(std::make_shared<RendererPanel>(*this));
    AddPanel(std::make_shared<HierarchyPanel>(*this));
    AddPanel(std::make_shared<PropertyPanel>(*this));
    AddPanel(m_viewer);
    AddPanel(std::make_shared<AssetInspector>(*this));
    AddPanel(std::make_shared<RenderGraphViewer>(*this));
#if !USING(PLATFORM_WASM)
    AddPanel(std::make_shared<FileSystemPanel>(*this));
#endif

    // @TODO: refactor this at some point
    m_shortcuts[SHORT_CUT_SAVE_AS] = {
        "Save As..",
        "Ctrl+Shift+S",
        [&]() {
            this->BufferCommand(std::make_shared<SaveProjectCommand>(true));
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

    // @TODO: proper key mapping
    std::map<std::string_view, KeyCode> keyMapping = {
        { "A", KeyCode::KEY_A },
        { "B", KeyCode::KEY_B },
        { "C", KeyCode::KEY_C },
        { "D", KeyCode::KEY_D },
        { "E", KeyCode::KEY_E },
        { "F", KeyCode::KEY_F },
        { "G", KeyCode::KEY_G },
        { "H", KeyCode::KEY_H },
        { "I", KeyCode::KEY_I },
        { "J", KeyCode::KEY_J },
        { "K", KeyCode::KEY_K },
        { "L", KeyCode::KEY_L },
        { "M", KeyCode::KEY_M },
        { "N", KeyCode::KEY_N },
        { "O", KeyCode::KEY_O },
        { "P", KeyCode::KEY_P },
        { "Q", KeyCode::KEY_Q },
        { "R", KeyCode::KEY_R },
        { "S", KeyCode::KEY_S },
        { "T", KeyCode::KEY_T },
        { "U", KeyCode::KEY_U },
        { "V", KeyCode::KEY_V },
        { "W", KeyCode::KEY_W },
        { "X", KeyCode::KEY_X },
        { "Y", KeyCode::KEY_Y },
        { "Z", KeyCode::KEY_Z },
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

void EditorLayer::OnAttach() {
    ImNodes::CreateContext();

    auto handle = AssetRegistry::GetSingleton().FindByPath<ImageAsset>(
        "@persist://textures/checkerboard");
    if (handle.is_some()) {
        context.checkerboard = handle.unwrap_unchecked().Wait();
    }

    m_app->GetInputManager()->PushInputHandler(this);

    for (auto& panel : m_panels) {
        panel->OnAttach();
    }
}

void EditorLayer::OnDetach() {
    [[maybe_unused]] auto handler = m_app->GetInputManager()->PopInputHandler();
    DEV_ASSERT(handler == this);

    ImNodes::DestroyContext();
}

void EditorLayer::AddPanel(std::shared_ptr<EditorItem> p_panel) {
    m_panels.emplace_back(p_panel);
}

void EditorLayer::SelectEntity(ecs::Entity p_selected) {
    m_selected = p_selected;
    // @TODO: fix this part
    auto scene = m_app->GetSceneManager()->GetActiveScene();
    scene->m_selected = m_selected;
}

void EditorLayer::DockSpace(Scene* p_scene) {
    ImGui::GetMainViewport();

    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
        window_flags |= ImGuiWindowFlags_NoBackground;
    }

    if (!opt_padding) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    if (!opt_padding) {
        ImGui::PopStyleVar();
    }

    ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    m_menu_bar->Update(p_scene);

    ImGui::End();
    return;
}

void EditorLayer::OnUpdate(float p_timestep) {
    context.timestep = p_timestep;

    // Scene* scene = SceneManager::GetSingleton().GetScenePtr();
    // Scene* scene = nullptr;
}

void EditorLayer::OnImGuiRender() {
    // @TODO: DO NOT Request SCENE here
    Scene* scene = m_app->GetSceneManager()->GetActiveScene().get();

    FlushInputEvents();

    DockSpace(scene);
    for (auto& it : m_panels) {
        it->Update(scene);
    }
    FlushCommand(scene);
}

void EditorLayer::FlushInputEvents() {
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
                    shortcut.executeFunc();
                    break;
                }
            }
        }
    }

    m_buffered_events.clear();
}

HandleInputResult EditorLayer::HandleInput(std::shared_ptr<InputEvent> p_input_event) {
    m_buffered_events.emplace_back(std::move(p_input_event));
    return HandleInputResult::NotHandled;
}

// @TODO: these are associated with scene editor, move to scene editor
void EditorLayer::BufferCommand(std::shared_ptr<EditorCommandBase>&& p_command) {
    p_command->m_editor = this;
    m_command_buffer.emplace_back(std::move(p_command));
}

void EditorLayer::CommandInspectAsset(const Guid& p_guid) {
    auto command = std::make_shared<EditorInspectAssetCommand>(p_guid);
    BufferCommand(command);
}

void EditorLayer::CommandAddComponent(ComponentName p_type, ecs::Entity p_target) {
    auto command = std::make_shared<EditorCommandAddComponent>(p_type);
    command->target = p_target;
    BufferCommand(command);
}

void EditorLayer::CommandAddEntity(EntityType p_type, ecs::Entity p_parent) {
    auto command = std::make_shared<EditorCommandAddEntity>(p_type);
    command->m_parent = p_parent;
    BufferCommand(command);
}

void EditorLayer::CommandRemoveEntity(ecs::Entity p_target) {
    auto command = std::make_shared<EditorCommandRemoveEntity>(p_target);
    BufferCommand(command);
}

void EditorLayer::FlushCommand(Scene* p_scene) {
    while (!m_command_buffer.empty()) {
        auto task = m_command_buffer.front();
        m_command_buffer.pop_front();
        task->Execute(*p_scene);
    }
}

CameraComponent* EditorLayer::GetActiveCamera() {
    if (auto tab = m_viewer->GetActiveTab(); tab) {
        return &(tab->GetActiveCamera());
    }

    return nullptr;
}

}  // namespace cave
