#include "tile_map_editor_tool.h"

#include "engine/scene/scene.h"
#include "editor/editor_layer.h"
#include "editor/panels/viewer.h"
#include "editor/utility/imguizmo.h"

// @TODO: refactor
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/common_dvars.h"

namespace my {

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
}

void TileMapEditor::OnExit() {
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
