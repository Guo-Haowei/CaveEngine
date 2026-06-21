#include "SpriteAnimationEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/diagnostics/DebugIdAllocator.h"

#include "editor/EditorState.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"
#include "engine/private/ui/inputs.h"
#include "engine/private/ui/layout.h"
#include "editor/services/IconCache.h"

// @TODO: refactor
#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/assets/ImageAsset.h"
// #include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/view/ViewManager.h"

namespace cave {

using namespace ::cave::math;

namespace {

void ImageSourceDropTarget(IDocument& doc, uint64_t checkerboard) {
    auto asset = doc.handle<SpriteAnimationAsset>().Get();
    DEV_ASSERT(asset);

    ImGui::Text("Source Image");

    auto image_handle = asset->GetImageHandle();
    ImageAsset* image = image_handle.Get();

    Vec2f region_size(128, 128);
    ui::CenteredImage(image, region_size, checkerboard);

    if (auto _handle = DragDropTarget(AssetType::Image); _handle.is_some()) {
        asset->SetGuid(_handle.unwrap_unchecked().GetGuid());
    }
}

}  // namespace

SpriteAnimationEditor::SpriteAnimationEditor(EditorState& editor,
                                             DocId doc_id,
                                             SceneId scene_id)
    : ViewTabBase(editor, doc_id, scene_id, ViewDimension::Dim2)
    , debug_id_(MakeDebugId(this)) {

#if 0
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
#endif
}

void SpriteAnimationEditor::onCreate() {
    ViewTabBase::onCreate();
}

void SpriteAnimationEditor::onDestroy() {
    ViewTabBase::onDestroy();
}

void SpriteAnimationEditor::submitView() {
    ViewTabBase::submitView(false);
}

void SpriteAnimationEditor::drawUIImpl() {
    ViewRecord* view = view_manager_.resolve(view_id_);
    DEV_ASSERT(view);

    updateRect(view->display_rect_os);
    drawMainView(view->display_rect_os);

    submitView();
}

void SpriteAnimationEditor::drawAssetInspector(IDocument& doc) {
    auto sprite_animation = doc.handle<SpriteAnimationAsset>().Get();
    DEV_ASSERT(sprite_animation);

    auto image_handle = sprite_animation->GetImageHandle();

    IconCache& icons = editor_services_.iconCache();

    if (ImGui::BeginTabBar("##MyTabs1")) {
        if (ImGui::BeginTabItem("Animation")) {
            ImageSourceDropTarget(doc, icons.GetIconHandle(IconName::Checkerboard));
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    sprite_selector_.EditSprite(nullptr, nullptr);

    if (ImageAsset* image = image_handle.Get()) {
        ImGui::Separator();
        drawFrameSelector(*sprite_animation, *image);
    }

    ImGui::Separator();
    drawTimeLine(*sprite_animation);
}

void SpriteAnimationEditor::drawFrameSelector(SpriteAnimationAsset& anim, ImageAsset& image_asset) {

    // @TODO: refactor this, this is the same as ViewerTab::DrawToolBar
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
    const auto& button_active = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

    clip_name_.resize(128);
    ui::TextBox("name", clip_name_.data(), (uint32_t)clip_name_.size(), true);

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_SQUARE_PLUS "  Add Animation")) {
        const auto [w, h] = sprite_selector_.GetDim();
        const float inv_w = 1.0f / w;
        const float inv_h = 1.0f / h;
        const auto& frame_indices = sprite_selector_.GetSelections();
        std::vector<Box2> frames;
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

        if (!clip_name_.empty() && !frames.empty()) {
            anim.AddClip(std::move(clip_name_), std::move(frames));
            clip_name_.clear();
            sprite_selector_.ClearSelections();
        }
    }

    ImGui::PopStyleColor(3);
    // -------------

    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));

    ImGui::Dummy(ImVec2(8, 8));

    sprite_selector_.SelectSprite(image_asset, nullptr, nullptr);

    ImGui::PopStyleVar(2);

    ImGui::EndGroup();
}

static void SelectAnimation(SpriteAnimationAsset& anim) {

    int current_clip = -1;
    std::vector<const char*> clips;
    for (const auto& [key, value] : anim.GetClips()) {
        // if (key == animator->GetCurrentClip()) {
        //     current_clip = static_cast<int>(clips.size());
        // }
        clips.push_back(key.c_str());
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
        // animator->SetClip(clips[current_clip]);
    }
}
void SpriteAnimationEditor::drawTimeLine(SpriteAnimationAsset& anim) {
    SelectAnimation(anim);
#if 0
    constexpr int width = 300;

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, width);
    {
        SpriteAnimatorComponent* animator = m_tmp_scene->GetComponent<SpriteAnimatorComponent>(m_animator_id);
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
    SpriteAnimatorComponent* animator = m_tmp_scene->GetComponent<SpriteAnimatorComponent>(m_animator_id);
    DEV_ASSERT(animator);

    std::vector<const ToolBarButtonDesc*> buttons = {
        animator->IsPlaying() ? &m_pause_button : &m_play_button
    };

    DrawToolBar(buttons);

    ImGui::Columns(1);

    // time line
    float& playback = animator->GetPlaybackTimer();
    if (ImGui::SliderFloat("timeline", &playback.timer, playback.start, playback.end)) {
        animator->SetPlaying(true);
    }
#endif
}

#if 0
const std::vector<const ToolBarButtonDesc*> SpriteAnimationEditor::GetToolBarButtons() const {
    SpriteAnimatorComponent* animator = m_tmp_scene->GetComponent<SpriteAnimatorComponent>(m_animator_id);
    const bool is_playing = animator->IsPlaying();

    return { is_playing ? &m_pause_button : &m_play_button };
}
#endif

}  // namespace cave
