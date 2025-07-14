#include "tile_map_editor_tool.h"

#include "engine/assets/assets.h"
#include "engine/assets/tile_map_asset.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/scene.h"
#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/panels/viewer.h"
#include "editor/utility/imguizmo.h"

// @TODO: refactor
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/common_dvars.h"

namespace my {

#define TEMP_SCENE_NAME "tile_map_scene"

TileMapEditor::TileMapEditor(EditorLayer& p_editor, Viewer* p_viewer)
    : ITool(p_editor), m_viewer(p_viewer) {
    m_policy = ToolCameraPolicy::Only2D;
    m_asset_registry = m_editor.GetApplication()->GetAssetRegistry();
}

void TileMapEditor::Update(Scene*) {
    {
        int16_t x = -2;
        int16_t y = -8;
        auto a = TileMapLayer::Pack(x, y);
        auto [x1, y1] = TileMapLayer::Unpack(a);
        DEV_ASSERT(x1 == x);
        DEV_ASSERT(y1 == y);
    }

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

        auto& layer = layers[0];

        // process commands
        for (const auto& command : m_commands) {
            std::visit([this, &layer](auto&& cmd) {
                using T = std::decay_t<decltype(cmd)>;
                if constexpr (std::is_same_v<T, CommandAddTile>) {
                    auto ndc = m_viewer->CursorToNDC(cmd.cursor);
                    Point tile;
                    if (CursorToTile(cmd.cursor, tile)) {
                        layer.AddTile(tile.x, tile.y, 1);
                        LOG_OK("add {} {}", tile.x, tile.y);
                    }
                } else if constexpr (std::is_same_v<T, CommandEraseTile>) {
                    auto ndc = m_viewer->CursorToNDC(cmd.cursor);
                    Point tile;
                    if (CursorToTile(cmd.cursor, tile)) {
                        layer.EraseTile(tile.x, tile.y);
                        LOG_OK("remove {} {}", tile.x, tile.y);
                    }
                }
            },
                       command);
        }
    } while (0);

    m_commands.clear();
}

bool TileMapEditor::CursorToTile(const Vector2f& p_in, Point& p_out) const {
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
    m_tile_map_guid = p_guid;
    // @TODO: create a dummy scene

    auto handle = *(m_editor.GetApplication()->GetAssetRegistry()->FindByGuid(p_guid));
    auto asset = handle.Get<TileMapAsset>();
    auto meta = handle.GetMeta();

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
        for (int16_t y = 0; y < (int16_t)data.size(); ++y) {
            for (int16_t x = 0; x < (int16_t)data[0].size(); ++x) {
                if (data[y][x]) {
                    layer.AddTile(x, y, data[y][x]);
                }
            }
        }
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

struct Card {
    ImTextureID texture;
    int id;  // optional for unique label suffix
};

static int next_card_id = 0;

void TileMapEditor::DrawLayerOverview() {
    // Top "+ Add" Button
    // if (ImGui::Button("+ Add Card")) {
    //    // Add a new card with dummy texture
    //    cards.push_back({ /* texture = */ nullptr, next_card_id++ });
    //}

    // Child container for scrollable card list
    ImGui::BeginChild("CardContainer", ImVec2(0, 400), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
    std::vector<Card> cards{};
    cards.resize(3);

    auto checkerboard = m_asset_registry->FindByPath<ImageAsset>("@res://images/checkerboard.png").value().Get();
    DEV_ASSERT(checkerboard);

    for (int i = 0; i < cards.size(); ++i) {
        ImGui::PushID(i);

        ImGui::BeginGroup();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));

        ImGui::BeginGroup();

        ImGui::Image(checkerboard->gpu_texture->GetHandle(), ImVec2(64, 64));
        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::Text("Card #%d", cards[i].id);  // You can put more info here

        // if (ImGui::Button("Delete")) {
        //     cards.erase(cards.begin() + i);
        //     ImGui::PopStyleVar(2);
        //     ImGui::EndGroup();
        //     ImGui::EndGroup();
        //     ImGui::PopID();
        //     //break;  // Must break because vector has changed
        // }

        ImGui::EndGroup();
        ImGui::EndGroup();
        ImGui::Separator();

        ImGui::PopStyleVar(2);
        ImGui::PopID();
        ImGui::EndGroup();
    }

    ImGui::EndChild();
}

void TileMapEditor::DrawAssetInspector() {
    // @TODO: draw layers with drop regions
    float full_width = ImGui::GetContentRegionAvail().x;
    constexpr float layer_tab_width = 360.0f;  // left panel fixed width
    constexpr float sprite_tab_width = 360.0f;
    [[maybe_unused]] const float main_width = full_width - layer_tab_width - sprite_tab_width - ImGui::GetStyle().ItemSpacing.x;

    ImGui::BeginChild("LayerTab", ImVec2(layer_tab_width, 0), true);
    DrawLayerOverview();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("SpriteTab", ImVec2(sprite_tab_width, 0), true);
    ImGui::Text("???");
    ImGui::EndChild();
}

}  // namespace my
