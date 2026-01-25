#include "EditorState.h"

#include "engine/private/debugger/profiler.h"
#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/framework/ViewportManager.h"

// @TODO: refactor
#include <imgui/imgui_internal.h>
#include <imnodes/imnodes.h>

#include "engine/private/assets/image_asset.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/renderer/graphics_manager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/framework/ISceneManager.h"
#include "engine/private/runtime/framework/ScriptManager.h"
#include "engine/private/ui/layout.h"

#include "editor/document/document.h"
#include "editor/EditorCommand.h"
#include "editor/EditorDvars.h"
#include "editor/panels/AssetInspector.h"
#include "editor/panels/FileSystemPanel.h"
#include "editor/panels/HierarchyPanel.h"
#include "editor/panels/LogPanel.h"
#include "editor/panels/MenuBar.h"
#include "editor/panels/PropertyPanel.h"
#include "editor/panels/RenderGraphViewer.h"
#include "editor/panels/RendererPanel.h"
#include "editor/viewer/Viewer.h"
#include "editor/viewer/ViewerTab.h"
#include "editor/widgets/Image.h"

namespace cave {

EditorState::EditorState(IApplication& p_app)
    : AppState(p_app)
    , m_shortcut_manager(*this) {

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
}

void EditorState::OnEnter(const StateRequest& p_args) {
    unused(p_args);

    ImNodes::CreateContext();

    auto handle = AssetRegistry::GetSingleton().FindByPath<ImageAsset>("@persist://textures/checkerboard");
    if (handle.is_some()) {
        context.checkerboard = handle.unwrap_unchecked().Wait();
    }

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
    m_app.GetViewportManager()->ClearViewport();

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

    DockSpace();
    for (auto& it : m_panels) {
        it->Update(p_timestep);
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
    m_request = Some(StateRequest{ AppStateId::GameRuntime });
}

void EditorState::AddPanel(std::shared_ptr<EditorItem> p_panel) {
    m_panels.emplace_back(p_panel);
}

void EditorState::DockSpace() {
    CAVE_PROFILE_EVENT();

    ui::DockSpace({
        "DockSpace Demo",
        [this]() { m_menu_bar->Update(0.0f); },
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
