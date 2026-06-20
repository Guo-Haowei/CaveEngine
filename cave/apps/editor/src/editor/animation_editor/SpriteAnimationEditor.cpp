#include "SpriteAnimationEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

#include "editor/EditorState.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"
#include "engine/private/ui/inputs.h"
#include "engine/private/ui/layout.h"
#include "editor/utility/ImGuizmo.h"

namespace cave {

#if 0
SpriteAnimationEditor::SpriteAnimationEditor(EditorState& p_editor, Viewer& p_viewer)
    : 
    , m_sprite_selector(SpriteSelector::SelectionMode::Multi) {

    // @TODO:
    // ICON_FA_FORWARD;
    // ICON_FA_BACKWARD;
    m_play_button = { ICON_FA_PLAY, "Play animation",
                      [&]() {
                          SpriteAnimatorComponent* animator = m_tmp_scene->GetComponent<SpriteAnimatorComponent>(m_animator_id);
                          if (DEV_VERIFY(animator)) {
                              animator->SetPlaying(true);
                          }
                      } };
    m_pause_button = { ICON_FA_PAUSE, "Pause animation",
                       [&]() {
                           SpriteAnimatorComponent* animator = m_tmp_scene->GetComponent<SpriteAnimatorComponent>(m_animator_id);
                           if (DEV_VERIFY(animator)) {
                               animator->SetPlaying(false);
                           }
                       } };
}

void SpriteAnimationEditor::OnCreateInternal(const Guid& p_guid) {
    DEV_ASSERT(0);
    unused(p_guid);
#if 0
    m_document = std::make_shared<SpriteAnimationDocument>(p_guid);

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApp().GetSceneRegistry());
    DEV_ASSERT(scene_manager);

    {
        auto scene = std::make_unique<Scene>();
        auto root = EntityFactory::CreateTransformEntity(*scene, "sprite_animation_test_scene");
        scene->m_root = root;

        auto id = EntityFactory::CreateTransformEntity(*scene, "animation_test");
        scene->AttachChild(id);

        scene->Create<SpriteRendererComponent>(id);

        SpriteAnimatorComponent& animator = scene->Create<SpriteAnimatorComponent>(id);
        animator.SetResourceGuid(p_guid);
        DEV_ASSERT(0);
    }

    // cache the id

    auto view = m_tmp_scene->View<SpriteAnimatorComponent>();
    for (const auto [id, _] : view) {
        DEV_ASSERT(!m_animator_id.IsValid());
        m_animator_id = id;
    }
#endif
}

void SpriteAnimationEditor::OnDestroy() {
    m_tmp_scene = nullptr;  // decrease ref count
}

void SpriteAnimationEditor::OnActivateInternal() {
    DEV_ASSERT(0);
    // auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApp().GetSceneRegistry());
    // DEV_ASSERT(scene_manager);
    //// scene_manager->OpenTempScene(m_tmp_scene);
    // m_scene_manager;
}

const std::vector<const ToolBarButtonDesc*> SpriteAnimationEditor::GetToolBarButtons() const {
    SpriteAnimatorComponent* animator = m_tmp_scene->GetComponent<SpriteAnimatorComponent>(m_animator_id);
    const bool is_playing = animator->IsPlaying();

    return { is_playing ? &m_pause_button : &m_play_button };
}

void SpriteAnimationEditor::DrawMainView(const CameraComponent& p_camera) {
    ViewerTab::DrawMainView(p_camera);

    const Mat4f proj_view = p_camera.GetProjectionViewMatrix();

    const Vector2f& canvas_min = m_viewer.GetCanvasMin();
    const Vector2f& canvas_size = m_viewer.GetCanvasSize();

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(canvas_min.x, canvas_min.y, canvas_size.x, canvas_size.y);

    ImGuizmo::DrawAxes(proj_view);

    // m_document->FlushCommands();
}



#endif

}  // namespace cave
