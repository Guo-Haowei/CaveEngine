#include "tile_map_editor_tool.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

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

namespace my {

#define TEMP_SCENE_NAME "tile_map_scene"

struct SetTileCommand : public UndoCommand {
    Handle<TileMapAsset> handle;
    int layer_id;
    TileIndex index;
    TileData old_tile;
    TileData new_tile;

    bool Undo() override {
        if (TileMapAsset* tile_map = handle.Get(); tile_map) {
            if (TileMapLayer* layer = tile_map->GetLayer(layer_id); layer) {
                TileData dummy;
                layer->SetTile(index, old_tile, dummy);
                layer->IncRevision();
                return true;
            }
        }
        return false;
    }

    bool Redo() override {
        if (TileMapAsset* tile_map = handle.Get(); tile_map) {
            if (TileMapLayer* layer = tile_map->GetLayer(layer_id); layer) {
                TileData dummy;
                layer->SetTile(index, new_tile, dummy);
                layer->IncRevision();
                return true;
            }
        }
        return false;
    }

    bool MergeCommand(const UndoCommand* p_other) override {
        if (auto other = dynamic_cast<const SetTileCommand*>(p_other); other) {
            return other->index == index &&
                   other->layer_id == layer_id &&
                   other->new_tile == new_tile &&
                   other->old_tile == old_tile &&
                   other->handle.GetGuid() == handle.GetGuid();
        }
        return false;
    }
};

TileMapEditor::TileMapEditor(EditorLayer& p_editor, Viewer* p_viewer)
    : ITool(p_editor), m_viewer(p_viewer) {
    m_policy = ToolCameraPolicy::Only2D;
}

void TileMapEditor::UndoableSetTile(TileMapLayer& p_layer,
                                    int p_layer_id,
                                    TileIndex p_index,
                                    TileData p_new_tile) {

    TileData old_tile;
    TileResult result = p_layer.SetTile(p_index, p_new_tile, old_tile);
    if (result == TileResult::Noop) {
        return;
    }

    p_layer.IncRevision();

    auto cmd = std::make_shared<SetTileCommand>();
    cmd->handle = m_tile_map_handle;
    cmd->index = p_index;
    cmd->layer_id = p_layer_id;
    cmd->new_tile = p_new_tile;
    cmd->old_tile = old_tile;

    m_undo_stack.Submit(std::move(cmd));
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
        TileMapAsset* asset = handle.Get<TileMapAsset>();
        if (!asset) {
            break;
        }

        auto& layers = asset->GetAllLayers();
        if (layers.empty()) {
            break;
        }

        int layer_id = 0;
        auto& layer = layers[layer_id];

        // process commands
        for (const auto& command : m_commands) {
            std::visit([&](auto&& cmd) {
                using T = std::decay_t<decltype(cmd)>;
                if constexpr (std::is_same_v<T, CommandAddTile>) {
                    auto ndc = m_viewer->CursorToNDC(cmd.cursor);
                    TileIndex tile;
                    if (CursorToTile(cmd.cursor, tile)) {
                        UndoableSetTile(layer, layer_id, tile, TileData(1));
                    }
                } else if constexpr (std::is_same_v<T, CommandEraseTile>) {
                    auto ndc = m_viewer->CursorToNDC(cmd.cursor);
                    TileIndex tile;
                    if (CursorToTile(cmd.cursor, tile)) {
                        UndoableSetTile(layer, layer_id, tile, TileData::Empty());
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
    m_undo_stack.Clear();

    m_asset_registry = m_editor.GetApplication()->GetAssetRegistry();
    m_tile_map_guid = p_guid;
    m_checkerboard_handle = m_asset_registry->FindByPath<ImageAsset>("@res://images/checkerboard.png").value();
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
        tile_map_renderer->tile_map = p_guid;

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

#if 0
        const int grid_x = 3;
        const int grid_y = 2;

        const float dx = 1.0f / grid_x;
        const float dy = 1.0f / grid_y;

        for (int y = 0; y < grid_y; ++y) {
            for (int x = 0; x < grid_x; ++x) {
                const float u0 = x * dx;
                const float v0 = (y + 1) * dy;
                const float u1 = (x + 1) * dx;
                const float v1 = y * dy;

                sprite.frames.push_back(Rect(Vector2f(u0, v0), Vector2f(u1, v1)));
            }
        }

#endif
        return scene;
    });
}

void TileMapEditor::OnExit() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

    scene_manager->DeleteTemporaryScene(TEMP_SCENE_NAME);
}

void TileMapEditor::DrawLayerOverview(TileMapAsset& p_tile_map) {
    if (ImGui::Button(ICON_FA_SQUARE_PLUS " Add Layer")) {
        p_tile_map.AddLayer("untitled layer");
    }
    ImGui::Separator();

    auto& layers = p_tile_map.GetAllLayers();
    const int layer_count = static_cast<int>(layers.size());

    auto checkerboard = m_checkerboard_handle.Get();
    DEV_ASSERT(checkerboard);

    for (int i = 0; i < layer_count; ++i) {
        TileMapLayer& layer = layers[i];

        ImGui::PushID(i);

        ImGui::BeginGroup();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));

        ImGui::BeginGroup();

        ImGui::Dummy(ImVec2(8, 8));

        DrawInputText("layer", layer.name);

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_TRASH_CAN)) {
            LOG_WARN("TODO: DELETE");
        }

        // next line

        ImVec2 region_size(96, 96);
        ImVec2 image_size = region_size;

        uint64_t image_handle = 0;

        if (layer.sprite_guid.IsValid()) {
            if (auto handle = m_asset_registry->FindByGuid<ImageAsset>(layer.sprite_guid); handle) {
                if (auto asset = (*handle).Get(); asset) {
                    // want image width always the same
                    image_handle = asset->gpu_texture ? asset->gpu_texture->GetHandle() : 0;
                    image_size = ImVec2(static_cast<float>(asset->width),
                                        static_cast<float>(asset->height));
                }
            }
        }
        if (image_handle == 0) {
            image_handle = checkerboard->gpu_texture->GetHandle();
        }

        CenteredImage(image_handle, image_size, region_size);

        ImVec2 pos = ImGui::GetItemRectMin();
        ImGui::SetCursorScreenPos(pos);
        if (ImGui::InvisibleButton("##clickable_image", region_size)) {
            LOG_WARN("TODO: SELECT");
        }

        DragDropTarget(AssetType::Image, [&](AssetHandle& p_handle) {
            DEV_ASSERT(p_handle.GetMeta()->type == AssetType::Image);
            layer.sprite_guid = p_handle.GetGuid();
        });

        ImGui::Dummy(ImVec2(8, 8));

        ImGui::EndGroup();
        ImGui::Separator();

        ImGui::PopStyleVar(2);
        ImGui::PopID();
        ImGui::EndGroup();
    }
}

void TileMapEditor::DrawAssetInspector() {
    TileMapAsset* tile_map = m_tile_map_handle.Get();
    if (!tile_map) {
        return;
    }

    float full_width = ImGui::GetContentRegionAvail().x;
    constexpr float layer_tab_width = 360.0f;  // left panel fixed width
    constexpr float sprite_tab_width = 360.0f;
    [[maybe_unused]] const float main_width = full_width - layer_tab_width - sprite_tab_width - ImGui::GetStyle().ItemSpacing.x;

    ImGui::BeginChild("LayerTab", ImVec2(layer_tab_width, 0), true);

    if (ImGui::BeginTabBar("##MyTabs1")) {
        if (ImGui::BeginTabItem("Layer")) {
            DrawLayerOverview(*tile_map);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("SpriteTab", ImVec2(sprite_tab_width, 0), true);
    ImGui::Text("???");
    ImGui::EndChild();
}

}  // namespace my
