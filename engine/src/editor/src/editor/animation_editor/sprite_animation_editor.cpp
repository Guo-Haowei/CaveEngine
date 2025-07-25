#include "sprite_animation_editor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

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

    // @TODO:
    // ICON_FA_FORWARD;
    // ICON_FA_BACKWARD;
    m_play_button = { ICON_FA_PLAY, "Play animation",
                      [&]() {
                          AnimatorComponent* animator = m_tmp_scene->GetComponent<AnimatorComponent>(m_animator_id);
                          if (DEV_VERIFY(animator)) {
                              animator->SetPlaying(true);
                          }
                      } };
    m_pause_button = { ICON_FA_PAUSE, "Pause animation",
                       [&]() {
                           AnimatorComponent* animator = m_tmp_scene->GetComponent<AnimatorComponent>(m_animator_id);
                           if (DEV_VERIFY(animator)) {
                               animator->SetPlaying(false);
                           }
                       } };
}

void SpriteAnimationEditor::OnCreate(const Guid& p_guid) {
    ViewerTab::OnCreate(p_guid);
    m_document = std::make_shared<SpriteAnimationDocument>(p_guid);

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

    m_tmp_scene = scene_manager->CreateTempScene(p_guid, [&]() {
        auto scene = std::make_shared<Scene>();
        auto root = EntityFactory::CreateTransformEntity(*scene, "sprite_animation_test_scene");
        scene->m_root = root;

        auto id = EntityFactory::CreateTransformEntity(*scene, "animation_test");
        scene->AttachChild(id);

        scene->Create<SpriteRendererComponent>(id);

        AnimatorComponent& animator = scene->Create<AnimatorComponent>(id);
        animator.SetResourceGuid(p_guid);

        return scene;
    });

    // cache the id

    auto view = m_tmp_scene->View<AnimatorComponent>();
    for (const auto [id, _] : view) {
        DEV_ASSERT(!m_animator_id.IsValid());
        m_animator_id = id;
    }
}

void SpriteAnimationEditor::OnDestroy() {
    m_tmp_scene = nullptr;  // decrease ref count
}

void SpriteAnimationEditor::OnActivate() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);
    scene_manager->OpenTempScene(m_tmp_scene);
}

const std::vector<const ToolBarButtonDesc*> SpriteAnimationEditor::GetToolBarButtons() const {
    AnimatorComponent* animator = m_tmp_scene->GetComponent<AnimatorComponent>(m_animator_id);
    const bool is_playing = animator->IsPlaying();

    return { is_playing ? &m_pause_button : &m_play_button };
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

void SpriteAnimationEditor::ImageSourceDropTarget() {
    auto asset = m_document->GetHandle<SpriteAnimationAsset>().Get();
    DEV_ASSERT(asset);

    ImGui::Text("Source Image");

    ImVec2 region_size(128, 128);

    auto image_handle = asset->GetImageHandle();
    ImageAsset* image = image_handle.Get();

    auto checkerboard = m_editor.context.checkerboard;

    CenteredImage(image, region_size, checkerboard->gpu_texture->GetHandle());

    if (auto _handle = DragDropTarget(AssetType::Image); _handle.is_some()) {
        asset->SetGuid(_handle.unwrap_unchecked().GetGuid());
    }
}

void SpriteAnimationEditor::DrawFrameSelector(ImageAsset& p_image_asset) {
    // @TODO: refactor this, this is the same as ViewerTab::DrawToolBar
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
    const auto& button_active = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

    DrawInputText("name", m_clip_name, 80, 160, false);

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_SQUARE_PLUS "  Add Animation")) {
        Handle<SpriteAnimationAsset> handle = m_document->GetHandle<SpriteAnimationAsset>();
        if (auto anim = handle.Get(); anim) {
            Handle<ImageAsset> image_handle = anim->GetImageHandle();
            if (auto image = image_handle.Get()) {
                const auto [w, h] = m_sprite_selector.GetDim();
                const float inv_w = 1.0f / w;
                const float inv_h = 1.0f / h;
                const auto& frame_indices = m_sprite_selector.GetSelections();
                std::vector<Rect> frames;
                frames.reserve(frame_indices.size());
                for (const auto [x, y] : frame_indices) {
#if 0
                    const float u0 = (x + 0) * inv_w;
                    const float v0 = (y + 0) * inv_h;
                    const float u1 = (x + 1) * inv_w;
                    const float v1 = (y + 1) * inv_h;
#else
                    const float u0 = (x + 0) * inv_w;
                    const float v0 = (y + 1) * inv_h;
                    const float u1 = (x + 1) * inv_w;
                    const float v1 = (y + 0) * inv_h;
#endif

                    frames.push_back({ { u0, v0 }, { u1, v1 } });
                }

                if (!m_clip_name.empty() && !frames.empty()) {
                    anim->AddClip(std::move(m_clip_name), std::move(frames));
                    m_clip_name.clear();
                    m_sprite_selector.ClearSelections();

                    m_document->SetDirty();
                }
            }
        }
    }

    ImGui::PopStyleColor(3);
    // -------------

    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));

    ImGui::Dummy(ImVec2(8, 8));

    m_sprite_selector.SelectSprite(p_image_asset, nullptr, nullptr);

    ImGui::PopStyleVar(2);

    ImGui::EndGroup();
}

void SpriteAnimationEditor::DrawTimeLine() {
    constexpr int width = 300;

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, width);
    {
        AnimatorComponent* animator = m_tmp_scene->GetComponent<AnimatorComponent>(m_animator_id);
        DEV_ASSERT(animator);

        int current_clip = -1;
        std::vector<const char*> clips;
        Handle<SpriteAnimationAsset> handle = m_document->GetHandle<SpriteAnimationAsset>();
        if (auto anim = handle.Get(); anim) {
            for (const auto& [key, value] : anim->GetClips()) {
                if (key == animator->GetCurrentClip()) {
                    current_clip = static_cast<int>(clips.size());
                }
                clips.push_back(key.c_str());
            }
        }

        const int old_clip = current_clip;

        const char* current_item = current_clip == -1 ? "select clip ..." : clips[current_clip];
        const int clip_count = static_cast<int>(clips.size());
        if (ImGui::BeginCombo("Clips", current_item)) {
            for (int n = 0; n < clip_count; ++n) {
                const bool is_selected = (current_clip == n);
                if (ImGui::Selectable(clips[n], is_selected)) {
                    current_clip = n;
                }

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (old_clip != current_clip) {
            LOG_OK("Set clip to {}", clips[current_clip]);
            animator->SetClip(clips[current_clip]);
        }
    }

    ImGui::NextColumn();
    AnimatorComponent* animator = m_tmp_scene->GetComponent<AnimatorComponent>(m_animator_id);
    DEV_ASSERT(animator);

    std::vector<const ToolBarButtonDesc*> buttons = {
        animator->IsPlaying() ? &m_pause_button : &m_play_button
    };

    DrawToolBar(buttons);

    ImGui::Columns(1);

#if 0
    // time line
    float& playback = animator->GetPlaybackTimer();
    if (ImGui::SliderFloat("timeline", &playback.timer, playback.start, playback.end)) {
        animator->SetPlaying(true);
    }
#endif
}

void SpriteAnimationEditor::DrawAssetInspector() {
    auto sprite_animation = m_document->GetHandle<SpriteAnimationAsset>().Get();
    DEV_ASSERT(sprite_animation);

    auto image_handle = sprite_animation->GetImageHandle();

    std::vector<AssetChildPanel> descs = {
        {
            "LayerOverview",
            360,
            [&]() {
                if (ImGui::BeginTabBar("##MyTabs1")) {
                    if (ImGui::BeginTabItem("Animation")) {
                        ImageSourceDropTarget();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            },
        },
        {
            "SpriteTab",
            360,
            [&]() {
                m_sprite_selector.EditSprite(nullptr, nullptr);
            },
        },
        {
            "PaintTab",
            600,
            [&]() {
                ImageAsset* image = image_handle.Get();
                if (image) {
                    DrawFrameSelector(*image);
                }
            },
        },
        {
            "TileLine",
            0,
            [&]() {
                DrawTimeLine();
            },
        },
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    DrawContents(full_width, descs);
}

Document& SpriteAnimationEditor::GetDocument() const {
    return *m_document.get();
}

bool SpriteAnimationEditor::HandleInput(const InputEvent* p_input_event) {
    unused(p_input_event);
    return false;
}

const CameraComponent& SpriteAnimationEditor::GetActiveCameraInternal() const {
    return *m_camera.get();
}

}  // namespace cave
