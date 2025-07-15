#include "tile_map_editor.h"

#include "tile_map_command.h"

#include "engine/assets/assets.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/scene.h"
#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/widget.h"
#include "editor/panels/viewer.h"
#include "editor/utility/imguizmo.h"

// @TODO: refactor
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/common_dvars.h"

namespace cave {

#define TEMP_SCENE_NAME "tile_map_scene"

TileMapEditor::TileMapEditor(EditorLayer& p_editor, Viewer* p_viewer)
    : ITool(p_editor), m_viewer(p_viewer) {
    m_policy = ToolCameraPolicy::Only2D;
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

void TileMapEditor::Update(Scene*) {
    const CameraComponent& camera = m_viewer->GetActiveCamera();
    const Matrix4x4f proj_view = camera.GetProjectionViewMatrix();

    const Vector2f& canvas_min = m_viewer->GetCanvasMin();
    const Vector2f& canvas_size = m_viewer->GetCanvasSize();

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(canvas_min.x, canvas_min.y, canvas_size.x, canvas_size.y);

    Matrix4x4f identity(1.0f);
    ImGuizmo::DrawGrid(proj_view, identity, 10.0f, ImGuizmo::GridPlane::XY);

    do {
        auto res = m_editor.GetApplication()->GetAssetRegistry()->FindByGuid(m_tile_map_guid);
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
                    auto ndc = m_viewer->CursorToNDC(p_cmd.cursor);
                    TileIndex tile;
                    if (CursorToTile(p_cmd.cursor, tile)) {
                        if (auto cmd = SetTileCommand::AddTile(*tile_map, tile, 1); cmd) {
                            cmd->SetHandle(std::move(handle));
                            m_undo_stack.Submit(cmd);
                        }
                    }
                } else if constexpr (std::is_same_v<T, CommandEraseTile>) {
                    auto ndc = m_viewer->CursorToNDC(p_cmd.cursor);
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
    auto res = m_viewer->CursorToNDC(p_in);
    if (!res) {
        return false;
    }
    auto ndc_2 = *res;
    Vector4f ndc{ ndc_2.x, ndc_2.y, 0.0f, 1.0f };

    CameraComponent& cam = m_viewer->GetActiveCamera();
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

void TileMapEditor::OnEnter(const Guid& p_guid) {
    Reset();

    m_asset_registry = m_editor.GetApplication()->GetAssetRegistry();
    m_tile_map_guid = p_guid;
    m_tile_map_handle = m_asset_registry->FindByGuid(m_tile_map_guid).value();

    [[maybe_unused]] auto asset = m_tile_map_handle.Get();
    [[maybe_unused]] auto meta = m_tile_map_handle.GetMeta();

    m_title = std::format("tilemap ({})", meta->path);

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

    scene_manager->OpenTemporaryScene(TEMP_SCENE_NAME, [&]() {
        auto scene = std::make_shared<Scene>();
        auto root = scene->CreateTransformEntity(TEMP_SCENE_NAME);
        scene->m_root = root;

        // test code, remember to take out
        auto id = scene->CreateTileMapEntity("tile_map");
        scene->AttachChild(id);

        TileMapRenderer* tile_map_renderer = scene->GetComponent<TileMapRenderer>(id);
        tile_map_renderer->SetTileMap(p_guid);

#if 0
        // clang-format off
        const std::vector<std::vector<int>> data = {
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, },
            { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
            { 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
            { 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
            { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, },
            { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, },
            { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, },
            { 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
        };
        // clang-format on

        // @HACK
        TileMapLayer& layer = asset->AddLayer("default");
        const int16_t w = static_cast<int16_t>(data[0].size());
        const int16_t h = static_cast<int16_t>(data.size());
        for (int16_t i = 0; i < h; ++i) {
            int16_t y = h - 1 - i;
            for (int16_t x = 0; x < w; ++x) {
                int cell = data[i][x];
                if (cell) {
                    layer.SetTile({ x, y }, TileData(cell));
                }
            }
        }
        layer.IncRevision();
#endif
        return scene;
    });
}

void TileMapEditor::OnExit() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

    scene_manager->DeleteTemporaryScene(TEMP_SCENE_NAME);
}

}  // namespace cave
