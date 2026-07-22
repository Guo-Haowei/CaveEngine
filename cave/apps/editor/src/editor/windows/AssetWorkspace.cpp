#include "AssetWorkspace.h"

#include "editor/workspaces/AnimationPanel.h"
#include "editor/workspaces/LogPanel.h"
#include "editor/workspaces/TileSetPanel.h"

#include "editor/EditorState.h"

namespace cave {

AssetWorkspace::AssetWorkspace(EditorState& editor)
    : EditorWindow(editor) {

    EngineServices& engine_services = editor.app().services();

    m_log = MakeOwner<LogPanel>(engine_services, editor.services());
    m_tile_set = MakeOwner<TileSetPanel>(engine_services, editor.services());
}

AssetWorkspace::~AssetWorkspace() = default;

void AssetWorkspace::onAttach() {
}

void AssetWorkspace::drawUIImpl() {
    if (!ImGui::BeginTabBar("##AssetTools")) {
        return;
    }

    if (ImGui::BeginTabItem("Output")) {
        m_log->draw();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Animation")) {
        // drawAnimationWorkspace();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Physics")) {
        // drawPhysicsWorkspace();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("TileMap")) {
        // drawTileMapWorkspace();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("TileSet")) {
        m_tile_set->draw();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}

}  // namespace cave
