#include "tile_map_editor.h"

#include "tile_map_editor_command.h"

#include "engine/assets/assets.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"
#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/widget.h"
#include "editor/viewer/viewer.h"
#include "editor/utility/imguizmo.h"

// @TODO: refactor
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/common_dvars.h"

namespace cave {

TileMapEditor::TileMapEditor(EditorLayer& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer) {
    m_policy = ToolCameraPolicy::Only2D;

    m_title = "TileMapEditor";

    m_commands.clear();
    m_current_layer_id = -1;
    m_tile_map_handle = Handle<TileMapAsset>();
    m_undo_stack.Clear();
}

TileMapAsset* TileMapEditor::GetActiveLayer() {
    if (TileMapAsset* tile_map = m_tile_map_handle.Get(); tile_map) {
        return tile_map;
    }
    return nullptr;
}

bool TileMapEditor::SetActiveLayer(int p_index) {
    m_current_layer_id = p_index;
    return true;
}

void TileMapEditor::Draw() {
    ViewerTab::Draw();

    const CameraComponent& camera = GetActiveCamera();
    const Matrix4x4f proj_view = camera.GetProjectionViewMatrix();

    const Vector2f& canvas_min = m_viewer.GetCanvasMin();
    const Vector2f& canvas_size = m_viewer.GetCanvasSize();

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(canvas_min.x, canvas_min.y, canvas_size.x, canvas_size.y);

    Matrix4x4f identity(1.0f);
    ImGuizmo::DrawGrid(proj_view, identity, 10.0f, ImGuizmo::GridPlane::XY);

    do {
        auto res = m_editor.GetApplication()->GetAssetRegistry()->FindByGuid(m_guid);
        if (!res) {
            break;
        }

        auto handle = *res;
        TileMapAsset* tile_map = handle.Get<TileMapAsset>();
        if (!tile_map) {
            break;
        }

        // process commands
        for (const auto& command : m_commands) {
            std::visit([&](auto&& p_cmd) {
                using T = std::decay_t<decltype(p_cmd)>;
                if constexpr (std::is_same_v<T, CommandAddTile>) {
                    auto ndc = m_viewer.CursorToNDC(p_cmd.cursor);
                    TileIndex tile;
                    if (CursorToTile(p_cmd.cursor, tile)) {
                        if (auto cmd = SetTileCommand::AddTile(*tile_map, tile, 1); cmd) {
                            cmd->SetHandle(std::move(handle));
                            m_undo_stack.Submit(cmd);
                        }
                    }
                } else if constexpr (std::is_same_v<T, CommandEraseTile>) {
                    auto ndc = m_viewer.CursorToNDC(p_cmd.cursor);
                    TileIndex tile;
                    if (CursorToTile(p_cmd.cursor, tile)) {
                        if (auto cmd = SetTileCommand::RemoveTile(*tile_map, tile); cmd) {
                            cmd->SetHandle(std::move(handle));
                            m_undo_stack.Submit(cmd);
                        }
                    }
                }
            },
                       command);
        }
    } while (0);

    m_commands.clear();
}

bool TileMapEditor::CursorToTile(const Vector2f& p_in, TileIndex& p_out) const {
    auto res = m_viewer.CursorToNDC(p_in);
    if (!res) {
        return false;
    }
    auto ndc_2 = *res;
    Vector4f ndc{ ndc_2.x, ndc_2.y, 0.0f, 1.0f };

    const CameraComponent& cam = GetActiveCamera();
    const auto inv_proj_view = glm::inverse(cam.GetProjectionViewMatrix());

    Vector4f position = inv_proj_view * ndc;
    position /= position.w;

    p_out.x = static_cast<int16_t>(std::floor(position.x));
    p_out.y = static_cast<int16_t>(std::floor(position.y));

    return true;
}

bool TileMapEditor::HandleInput(const std::shared_ptr<InputEvent>& p_input_event) {
    InputEvent* event = p_input_event.get();
    if (auto e = dynamic_cast<InputEventMouse*>(event); e) {
        if (!e->IsModiferPressed()) {
            if (e->IsButtonDown(MouseButton::LEFT)) {
                CommandAddTile command{ e->GetPos(), 1 };
                m_commands.push_back(command);
                return true;
            }
            if (e->IsButtonDown(MouseButton::RIGHT)) {
                CommandEraseTile command{ e->GetPos() };
                m_commands.push_back(command);
                return true;
            }
        }
    }

    return false;
}

void TileMapEditor::OnCreate(const Guid& p_guid) {
    m_asset_registry = m_editor.GetApplication()->GetAssetRegistry();
    m_guid = p_guid;
    m_tile_map_handle = m_asset_registry->FindByGuid(m_guid).value();

    [[maybe_unused]] auto asset = m_tile_map_handle.Get();
    [[maybe_unused]] auto meta = m_tile_map_handle.GetMeta();

    m_title = std::format("tilemap ({})", meta->path);

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

#define TEMP_SCENE_NAME "tile_map_scene"
    m_tmp_scene = scene_manager->OpenTemporaryScene(m_guid, [&]() {
        auto scene = std::make_shared<Scene>();
        auto root = EntityFactory::CreateTransformEntity(*scene, TEMP_SCENE_NAME);
        scene->m_root = root;

        // test code, remember to take out
        auto id = EntityFactory::CreateTileMapEntity(*scene, "tile_map");
        scene->AttachChild(id);

        TileMapRenderer* tile_map_renderer = scene->GetComponent<TileMapRenderer>(id);
        tile_map_renderer->SetTileMap(p_guid);
        return scene;
    });
}

void TileMapEditor::OnDestroy() {
    m_tmp_scene = nullptr;  // decrease ref count
}

void TileMapEditor::OnActivate() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);
    scene_manager->SetTmpScene(m_tmp_scene);
}

}  // namespace cave
