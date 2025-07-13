#include "tile_map_editor_tool.h"

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

    // process commands
    for (const auto& command : m_commands) {
        switch (command.type) {
            case TileMapEditCommand::INSERT:
                LOG_OK("insert");
                break;
            case TileMapEditCommand::ERASE:
                LOG_OK("erase");
                break;
            default:
                break;
        }
    }

    m_commands.clear();
}

bool TileMapEditor::HandleInput(const std::shared_ptr<InputEvent>& p_input_event) {
    InputEvent* event = p_input_event.get();
    if (auto e = dynamic_cast<InputEventMouse*>(event); e) {
        if (!e->IsModiferPressed()) {
            if (e->IsButtonDown(MouseButton::LEFT)) {
                TileMapEditCommand command;
                command.type = TileMapEditCommand::INSERT;
                command.cursor = e->GetPos();
                m_commands.push_back(command);
                return true;
            }
            if (e->IsButtonDown(MouseButton::RIGHT)) {
                TileMapEditCommand command;
                command.type = TileMapEditCommand::ERASE;
                command.cursor = e->GetPos();
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

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

    scene_manager->OpenTemporaryScene(TEMP_SCENE_NAME, [&p_guid]() {
        auto scene = std::make_shared<Scene>();
        auto root = scene->CreateTransformEntity(TEMP_SCENE_NAME);
        scene->m_root = root;

        // clang-format off
        [[maybe_unused]]
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

        // test code, remember to take out
        auto id = scene->CreateTileMapEntity("tile_map");
        scene->AttachChild(id);

        TileMapRenderer* tile_map_renderer = scene->GetComponent<TileMapRenderer>(id);
        tile_map_renderer->tile_map = p_guid;

        #if 0
        tileMap->FromArray(data);

        auto& sprite = tileMap->m_sprite;

        auto res = (m_assetRegistry->FindByPath("@res://images/tiles.png")).value().Wait<ImageAsset>();

        sprite.texture = (*res).get();

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

#if 0
    void Process(Scene& p_scene, const CameraComponent& p_camera) override {

        if (!m_viewer.m_focused || !m_viewer.m_input_state.ndc) {
            return;
        }

        Vector2f ndc_2 = *m_viewer.m_input_state.ndc;

        auto selected = m_viewer.m_editor.GetSelectedEntity();

        unused(ndc_2);
        unused(p_scene);
        unused(p_camera);
        unused(selected);

        TileMapComponent* tile_map = p_scene.GetComponent<TileMapComponent>(selected);

        if (!tile_map) {
            return;
        }

        Vector4f ndc{ ndc_2.x, ndc_2.y, 0.0f, 1.0f };
        const auto inv_proj_view = glm::inverse(p_camera.GetProjectionViewMatrix());
        Vector4f position = inv_proj_view * ndc;
        position /= position.w;

        int tile_x = static_cast<int>(position.x);
        int tile_y = static_cast<int>(position.y);

        // erase if right button down
        int value = 0;
        if (m_viewer.m_input_state.buttons[std::to_underlying(MouseButton::LEFT)]) {
            value = m_viewer.m_editor.context.selected_tile;
            if (value < 0 || value > 5) {
                value = 0;
            }
        }
        tile_map->SetTile(tile_x, tile_y, value);
#endif

}  // namespace my
