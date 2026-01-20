#include "editor_state.h"

#include "engine/debugger/profiler.h"
#include "engine/runtime/application.h"
#include "engine/runtime/imgui_manager.h"

// @TODO: refactor
#include "editor_layer.h"

namespace cave {

void EditorState::OnEnter(const StateRequest& p_args) {
    unused(p_args);

    m_editorLayer = std::make_unique<EditorLayer>();
    AttachLayer(m_editorLayer.get());

    for (auto& layer : m_layers) {
        layer->m_app = &m_app;
        layer->OnAttach();
        LOG("[Runtime] layer '{}' attached!", layer->GetName());
    }
}

void EditorState::OnExit() {
    for (auto& layer : m_layers) {
        layer->OnDetach();
        LOG("[Runtime] layer '{}' detached!", layer->GetName());
    }
    m_layers.clear();
}

void EditorState::Tick(float p_timestep) {
    // layer should set active scene
    // update layers from back to front
    for (int i = (int)m_layers.size() - 1; i >= 0; --i) {
        m_layers[i]->OnUpdate(p_timestep);
    }

    ImguiManager* m_imgui_manager = m_app.GetImguiManager();
    //ModeManager& m_mode_manager = m_app.GetModeManager();

    // @TODO: refactor this
    if (m_imgui_manager) {
        {
            CAVE_PROFILE_EVENT("ImGuiManager::BeginFrame");
            m_imgui_manager->BeginFrame();
        }

        for (int i = (int)m_layers.size() - 1; i >= 0; --i) {
            m_layers[i]->OnImGuiRender();
        }

        {
            CAVE_PROFILE_EVENT("ImGui::Render");
            ImGui::Render();
        }
    }

    //const GameMode game_mode = m_mode_manager.GetMode();
    // change game mode from here

    // @TODO: set mode here
    //std::shared_ptr<Scene> scene = m_scene_manager->GetActiveScene();

    //if (scene && game_mode == GameMode::Gameplay) {
    //    m_script_manager->Update(*scene, p_timestep);
    //}
}

StateRequest EditorState::PopRequest() {
    return {};
}

// @TODO: refactor layer
void EditorState::AttachLayer(Layer* p_layer) {
    DEV_ASSERT(p_layer);

    p_layer->m_app = &m_app;
    p_layer->OnAttach();
    m_layers.emplace_back(p_layer);
}

void EditorState::DetachLayer(Layer* p_layer) {
    DEV_ASSERT(p_layer);

    auto it = std::find(m_layers.begin(), m_layers.end(), p_layer);
    if (it == m_layers.end()) {
        LOG_WARN("Layer '{}' not found");
        return;
    }

    m_layers.erase(it);
    p_layer->OnDetach();
    p_layer->m_app = nullptr;
}

GameLayer* EditorState::GetGameLayer() {
    return m_game_layer.get();
}

void EditorState::AttachGameLayer() {
    if (m_game_layer) {
        AttachLayer(m_game_layer.get());
    }
}

void EditorState::DetachGameLayer() {
    if (m_game_layer) {
        DetachLayer(m_game_layer.get());
    }
}


}  // namespace cave
