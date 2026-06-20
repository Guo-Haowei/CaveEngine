#include "AssetInspector.h"
#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/document/TileMapDocument.h"
#include "editor/services/DocumentService.h"
#include "editor/services/IconCache.h"
#include "editor/services/Workspace.h"
#include "editor/widgets/DragDrop.h"
#include "editor/widgets/Image.h"

// @TODO: remove private #include
#include "engine/private/ui/inputs.h"
#include "engine/private/ui/layout.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/SpriteAnimationAsset.h"

#include "editor/EditorState.h"

namespace cave {

using namespace ::cave::math;

AssetInspector::AssetInspector(EditorState& editor,
                               EditorServices& editor_services)
    : EditorWindow(editor)
    , editor_services_(editor_services) {
}
void AssetInspector::onAttach() {
    IconCache& icons = editor_services_.iconCache();
    checkerboard_ = icons.GetIconHandle(IconName::Checkerboard);
}

void AssetInspector::tileMapLayerOverview(TileMapAsset& p_tile_map) {
    if (ImGui::Button(ICON_FA_SQUARE_PLUS " Add Layer")) {
        // p_tile_map.AddLayer("untitled layer");
        LOG_WARN("TODO: Add layer");
    }
    ImGui::Separator();

    for (int layer_id = 0; layer_id < 1; ++layer_id) {
        TileMapAsset& layer = p_tile_map;
        const bool is_layer_selected = true;

        ImGui::PushID(layer_id);

        if (is_layer_selected) {
            auto& style = ImGui::GetStyle();
            auto& colors = style.Colors;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_FrameBgHovered]);
        }

        ImGui::BeginGroup();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));

        ImGui::BeginGroup();

        ImGui::Dummy(ImVec2(8, 8));

        // if (ui::TextBox("layer", layer.GetName().c_str())) {
        //     // @TODO: notify dirty
        // }

        ImGui::SameLine();

        const bool is_visible = layer.visible();
        const char* label = is_visible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
        if (ImGui::Button(label)) {
            layer.visible(!is_visible);
        }

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_TRASH_CAN)) {
            LOG_WARN("TODO: DELETE");
        }

        {

            const ImageAsset* image = nullptr;
            if (auto image_handle = layer.tileSetHandle().Get(); image_handle) {
                image = image_handle->handle().Get();
            }

            Vec2f region_size(128, 128);
            ui::CenteredImage(image, region_size, checkerboard_);

            if (ImGui::IsItemClicked()) {
                // tool->SetActiveLayer(layer_id);
            }

            // @TODO: make an asset drop region
            // accept same type of assets, show tooltips, etc
            if (auto _handle = DragDropTarget(AssetType::TileSet); _handle.is_some()) {
                layer.tileSetGuid(_handle.unwrap_unchecked().GetGuid());
            }
        }

        ImGui::Dummy(ImVec2(8, 8));

        ImGui::EndGroup();
        ImGui::Separator();

        ImGui::PopStyleVar(2);
        ImGui::PopID();
        ImGui::EndGroup();

        if (is_layer_selected) {
            ImGui::PopStyleColor();
        }
    }
}

void AssetInspector::drawTileMap(TileMapDocument& doc) {
    TileMapAsset* tile_map = doc.handle<TileMapAsset>().Get();
    DEV_ASSERT(tile_map);

    if (ImGui::BeginTabBar("##MyTabs1")) {
        if (ImGui::BeginTabItem("Layer")) {
            tileMapLayerOverview(*tile_map);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();

    TileSetAsset* tile_set = tile_map->tileSetHandle().Get();
    if (tile_set) {
        auto handle = tile_set->handle();
        const int column = tile_set->col();
        const int row = tile_set->row();
        if (auto image = handle.Get(); image) {
            tile_map_ctx_.sprite_selector.SelectSprite(*image, &column, &row);
        }
    }
}

static void DrawPhysicsTab(TileSetAsset& tile_set, SpriteSelector& sprite_selector) {
    int index = -1;
    if (auto selected = sprite_selector.GetSelections(); !selected.empty()) {
        auto [x, y] = selected.front();
        index = tile_set.col() * y + x;
    }

    ToolBarButtonDesc add_square_button_desc = { ICON_FA_SQUARE " Box", "Add box collider",
                                                 [&]() {
                                                     if (index >= 0 && tile_set.addBoxCollider(index)) {
                                                         LOG_OK("Box collider added for {}", index);
                                                     } else {
                                                         LOG_ERROR("Failed to add box collider for {}", index);
                                                     }
                                                 } };

    ToolBarButtonDesc add_polygon_button_desc = { ICON_FA_DRAW_POLYGON " Polygon", "Add polygon collider",
                                                  [&]() {
                                                      LOG_WARN("Not implemented");
                                                  } };

    ToolBarButtonDesc add_circle_button_desc = { ICON_FA_CIRCLE " Circle", "Add circle collider",
                                                 [&]() {
                                                     LOG_WARN("Not implemented");
                                                 } };

    std::vector<const ToolBarButtonDesc*> tool_bar = {
        &add_square_button_desc,
        &add_polygon_button_desc,
        &add_circle_button_desc,
    };

    DrawToolBar(tool_bar);
    ImGui::Separator();
}

void AssetInspector::drawTileSet(IDocument& doc) {
    TileSetAsset* tile_set = doc.handle<TileSetAsset>().Get();
    DEV_ASSERT(tile_set);
    if (!tile_set) {
        return;
    }

    auto& sprite_selector = tile_map_ctx_.sprite_selector;
    {
        int column = tile_set->col();
        int row = tile_set->row();
        if (sprite_selector.EditSprite(&column, &row)) {
            tile_set->col(column);
            tile_set->row(row);
        }
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("TileSetPhysics")) {
        if (ImGui::BeginTabItem("Physics Layer")) {
            DrawPhysicsTab(*tile_set, sprite_selector);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    if (tile_set) {
        auto handle = tile_set->handle();
        const int column = tile_set->col();
        const int row = tile_set->row();
        if (auto image = handle.Get(); image) {
            sprite_selector.SelectSprite(*image, &column, &row);
        }
    }
}

static void ImageSourceDropTarget(IDocument& doc, uint64_t checkerboard) {
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

static void DrawFrameSelector(SpriteAnimationAsset& anim,
                              ImageAsset& image_asset,
                              SpriteAnimationContext& ctx) {

    // @TODO: refactor this, this is the same as ViewerTab::DrawToolBar
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
    const auto& button_active = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

    auto& clip_name = ctx.clip_name;
    clip_name.resize(128);
    ui::TextBox("name", clip_name.data(), (uint32_t)clip_name.size(), true);

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_SQUARE_PLUS "  Add Animation")) {
        const auto [w, h] = ctx.sprite_selector.GetDim();
        const float inv_w = 1.0f / w;
        const float inv_h = 1.0f / h;
        const auto& frame_indices = ctx.sprite_selector.GetSelections();
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

        if (!clip_name.empty() && !frames.empty()) {
            anim.AddClip(std::move(clip_name), std::move(frames));
            clip_name.clear();
            ctx.sprite_selector.ClearSelections();
        }
    }

    ImGui::PopStyleColor(3);
    // -------------

    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0));

    ImGui::Dummy(ImVec2(8, 8));

    ctx.sprite_selector.SelectSprite(image_asset, nullptr, nullptr);

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

static void DrawTimeLine(SpriteAnimationAsset& anim) {
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

void AssetInspector::drawSpriteAnimation(IDocument& doc) {
    auto sprite_animation = doc.handle<SpriteAnimationAsset>().Get();
    DEV_ASSERT(sprite_animation);

    auto image_handle = sprite_animation->GetImageHandle();

    if (ImGui::BeginTabBar("##MyTabs1")) {
        if (ImGui::BeginTabItem("Animation")) {
            ImageSourceDropTarget(doc, checkerboard_);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    sprite_animation_ctx_.sprite_selector.EditSprite(nullptr, nullptr);

    if (ImageAsset* image = image_handle.Get()) {
        ImGui::Separator();
        DrawFrameSelector(*sprite_animation, *image, sprite_animation_ctx_);
    }

    ImGui::Separator();
    DrawTimeLine(*sprite_animation);
}

void AssetInspector::drawUIImpl() {
    DocId doc_id = editor_services_.workspace().focusedDoc();
    IDocument* doc = editor_services_.document().resolve(doc_id);
    if (!doc) {
        return;
    }

    if (auto tilemap = dynamic_cast<TileMapDocument*>(doc)) {
        drawTileMap(*tilemap);
        return;
    }

    IAsset* asset = doc->rawHandle().Get();
    switch (asset->GetType()) {
        case AssetType::TileMap: {
            drawTileMap(*static_cast<TileMapDocument*>(doc));
        } break;
        case AssetType::TileSet: {
            drawTileSet(*doc);
        } break;
        case AssetType::SpriteAnimation: {
            drawSpriteAnimation(*doc);
        } break;
        default:
            break;
    }
}

}  // namespace cave
