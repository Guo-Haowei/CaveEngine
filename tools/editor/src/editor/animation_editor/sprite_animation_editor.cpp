#include "sprite_animation_editor.h"

#include "engine/assets/image_asset.h"
#include "engine/input/input_event.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

#include "editor/document/document.h"
#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/widgets/widget.h"
#include "editor/viewer/viewer.h"
#include "editor/utility/imguizmo.h"

namespace cave {

SpriteAnimationEditor::SpriteAnimationEditor(EditorLayer& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer) {

    m_camera = std::make_unique<CameraComponent>();
    ViewerTab::CreateDefaultCamera2D(*m_camera.get());
}

void SpriteAnimationEditor::OnCreate(const Guid& p_guid) {
    ViewerTab::OnCreate(p_guid);
    m_document = std::make_shared<SpriteAnimationDocument>(p_guid);

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

    m_tmp_scene = scene_manager->OpenTemporaryScene(p_guid, [&]() {
        auto scene = std::make_shared<Scene>();
        auto root = EntityFactory::CreateTransformEntity(*scene, "sprite_animation_test_scene");
        scene->m_root = root;

        auto id = EntityFactory::CreateTransformEntity(*scene, "test_sprite");
        scene->AttachChild(id);

        auto test_image = AssetRegistry::GetSingleton().FindByPath<ImageAsset>("@res://player/player.png").unwrap();

        SpriteRenderer& sprite_renderer = scene->Create<SpriteRenderer>(id);
        sprite_renderer.SetImage(test_image.GetGuid());
        return scene;
    });
}

void SpriteAnimationEditor::OnDestroy() {
    m_tmp_scene = nullptr;  // decrease ref count
}

void SpriteAnimationEditor::OnActivate() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);
    scene_manager->SetTmpScene(m_tmp_scene);
}

void SpriteAnimationEditor::DrawMainView(const CameraComponent& p_camera) {
    ViewerTab::DrawMainView(p_camera);

    const Matrix4x4f proj_view = p_camera.GetProjectionViewMatrix();

    const Vector2f& canvas_min = m_viewer.GetCanvasMin();
    const Vector2f& canvas_size = m_viewer.GetCanvasSize();

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(canvas_min.x, canvas_min.y, canvas_size.x, canvas_size.y);

    ImGuizmo::DrawAxes(proj_view);

    // m_document->FlushCommands();
}

Document& SpriteAnimationEditor::GetDocument() const {
    return *m_document.get();
}

bool SpriteAnimationEditor::HandleInput(const InputEvent* p_input_event) {
    unused(p_input_event);
    return false;
}

const CameraComponent& SpriteAnimationEditor::GetActiveCameraInternal() const {
    DEV_ASSERT(m_camera);
    return *m_camera.get();
}

}  // namespace cave
