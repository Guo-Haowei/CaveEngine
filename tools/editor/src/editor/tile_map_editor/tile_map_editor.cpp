#include "tile_map_editor.h"

#include "engine/assets/assets.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"
#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/widget.h"
#include "editor/viewer/viewer.h"
#include "editor/utility/imguizmo.h"
#include "editor/tile_map_editor/tile_map_document.h"

// @TODO: refactor
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/common_dvars.h"

namespace cave {

TileMapEditor::TileMapEditor(EditorLayer& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer) {

    m_title = "TileMapEditor";

    m_camera = ViewerTab::CreateDefaultCamera2D();
}

void TileMapEditor::OnCreate(const Guid& p_guid) {
    m_document = std::make_shared<TileMapDocument>(p_guid, *this);

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

#define TEMP_SCENE_NAME "tile_map_scene"
    m_tmp_scene = scene_manager->OpenTemporaryScene(p_guid, [&]() {
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

    // @NOTE: shouldn't do it here,
    // move it do somewhere else
    m_document->FlushCommands();
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

Document& TileMapEditor::GetDocument() const {
    return *m_document.get();
}

bool TileMapEditor::HandleInput(const InputEvent* p_input_event) {
    if (auto e = dynamic_cast<const InputEventMouse*>(p_input_event); e) {
        if (!e->IsModiferPressed()) {
            if (e->IsButtonDown(MouseButton::LEFT)) {
                m_document->RequestAdd(e->GetPos(), TileId(1));
                return true;
            }
            if (e->IsButtonDown(MouseButton::RIGHT)) {
                m_document->RequestErase(e->GetPos());
                return true;
            }
        }
    }

    return false;
}

const CameraComponent& TileMapEditor::GetActiveCameraInternal() const {
    DEV_ASSERT(m_camera);
    return *m_camera.get();
}

}  // namespace cave
