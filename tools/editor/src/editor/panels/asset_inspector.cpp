#include "asset_inspector.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/assets.h"
#include "engine/assets/sprite_asset.h"
#include "engine/runtime/asset_registry.h"
#include "editor/editor_layer.h"
#include "editor/widget.h"
#include "editor/tools/tile_map_editor_tool.h"

namespace my {

AssetInspector::AssetInspector(EditorLayer& p_editor)
    : EditorWindow("Asset Inspector", p_editor) {
}

void AssetInspector::OnAttach() {
    m_asset_registry = m_editor.GetApplication()->GetAssetRegistry();
    m_checkerboard_handle = m_asset_registry->FindByPath<ImageAsset>("@res://images/checkerboard.png").value();
}

void AssetInspector::TilePaint(SpriteAsset& p_sprite) {
    ImGui::Text("TileSet");

    auto& handle = p_sprite.GetHandle();
    if (!handle.IsReady()) {
        return;
    }

    const ImageAsset* image = handle.Get();
    DEV_ASSERT(image);

    const uint32_t width = p_sprite.GetWidth();
    const uint32_t height = p_sprite.GetHeight();
    if (!width || !height) {
        return;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 tile_size((float)width, (float)height);

    ImGui::InvisibleButton("TileClickable", tile_size);  // enables interaction
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();
    unused(hovered);
    unused(clicked);

    // Draw tileset
    draw_list->AddImage(
        image->gpu_texture->GetHandle(),
        cursor,
        cursor + tile_size,
        ImVec2(0, 0), ImVec2(1, 1),
        IM_COL32(255, 255, 255, 255));

    const int num_col = p_sprite.GetCol();
    const int num_row = p_sprite.GetRow();

    const float cell_w = static_cast<float>(width) / num_col;
    const float cell_h = static_cast<float>(height) / num_row;

    if (hovered && clicked) {
        ImVec2 mouse_pos = ImGui::GetMousePos();

        int local_x = (int)((mouse_pos.x - cursor.x) / cell_w);
        int local_y = (int)((mouse_pos.y - cursor.y) / cell_h);

        // Clamp to valid range
        if (local_x >= 0 && local_x < num_col && local_y >= 0 && local_y < num_row) {
            m_selected_x = local_x;
            m_selected_y = local_y;
        }
    }

    for (float dx = cell_w; dx < tile_size.x; dx += cell_w) {
        draw_list->AddLine(ImVec2(cursor.x + dx, cursor.y),
                           ImVec2(cursor.x + dx, cursor.y + tile_size.y),
                           IM_COL32(255, 255, 255, 255));
    }

    for (float dy = cell_h; dy < tile_size.y; dy += cell_h) {
        draw_list->AddLine(ImVec2(cursor.x, cursor.y + dy),
                           ImVec2(cursor.x + tile_size.x, cursor.y + dy),
                           IM_COL32(255, 255, 255, 255));
    }

    if (m_selected_x >= 0 && m_selected_y >= 0) {
        float cellW = tile_size.x / num_col;
        float cellH = tile_size.y / num_row;

        ImVec2 pMin = ImVec2(cursor.x + m_selected_x * cellW, cursor.y + m_selected_y * cellH);
        ImVec2 pMax = ImVec2(pMin.x + cellW, pMin.y + cellH);

        draw_list->AddRectFilled(pMin, pMax, IM_COL32(0, 255, 0, 100));  // green transparent overlay
        draw_list->AddRect(pMin, pMax, IM_COL32(0, 255, 0, 255));        // solid border

    } else {
    }
}

void AssetInspector::EditSprite(SpriteAsset& p_sprite) {
    if (ImGui::BeginTabBar("TileSetModes")) {
        if (ImGui::BeginTabItem("Setup")) {
            int column = p_sprite.GetCol();
            if (ImGui::InputInt("column", &column)) {
                p_sprite.SetCol(column);
            }
            int row = p_sprite.GetRow();
            if (ImGui::InputInt("row", &row)) {
                p_sprite.SetRow(row);
            }

            float scale = p_sprite.GetScale();
            if (ImGui::InputFloat("scale", &scale)) {
                p_sprite.SetScale(scale);
            }

            // ImGui::Checkbox("Use Texture Region", &use_region);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Select")) {
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

struct AssetChildPanel {
    const char* name;
    float width;
    std::function<void()> func;
};

static void DrawContents(float p_full_width, const std::vector<AssetChildPanel>& p_descs) {
    const int size = static_cast<int>(p_descs.size());
    float width_so_far = 0.0f;
    for (int i = 0; i < size; ++i) {
        const auto& desc = p_descs[i];
        const bool is_last = i + 1 == size;

        const float width = is_last ? p_full_width - width_so_far : desc.width;
        width_so_far += width;

        ImGui::BeginChild(desc.name, ImVec2(width, 0), true);
        desc.func();
        ImGui::EndChild();

        if (!is_last) {
            ImGui::SameLine();
        }
    }
}

void AssetInspector::InspectSprite(IAsset* p_asset) {
    auto sprite = dynamic_cast<SpriteAsset*>(p_asset);
    if (!sprite) {
        return;
    }

    std::vector<AssetChildPanel> descs = {
        { "ImageSource", 360.0f,
          [&]() {
              ImGui::Text("Image");
              const float w = 300;
              auto& handle = sprite->GetHandle();

              // @TODO: checker board
              if (handle.IsReady()) {
                  const ImageAsset* asset = handle.Get();
                  DEV_ASSERT(asset);
                  const float h = w / asset->width * asset->height;
                  ImVec2 size = ImVec2(w, h);

                  ImGui::Image(asset->gpu_texture->GetHandle(), size);
              } else {
                  ImVec2 size = ImVec2(w, w);
                  ImGui::InvisibleButton("DropTarget", size);
              }

              DragDropTarget(AssetType::Image, [&](AssetHandle& p_handle) {
                  DEV_ASSERT(p_handle.GetMeta()->type == AssetType::Image);
                  sprite->SetImage(p_handle.GetGuid());
              });
          } },
        { "SpriteEditor", 360.0f,
          [&]() { EditSprite(*sprite); } },
        { "TileSetPanel", 0.0f,
          [&]() { TilePaint(*sprite); } },
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    DrawContents(full_width, descs);
}

void AssetInspector::InspectTileMap(IAsset* p_asset) {
    auto tile_map = dynamic_cast<TileMapAsset*>(p_asset);
    if (!tile_map) {
        return;
    }

    std::vector<AssetChildPanel> descs = {
        { "LayerOverview", 360.0f,
          [&]() {
              if (ImGui::BeginTabBar("##MyTabs1")) {
                  if (ImGui::BeginTabItem("Layer")) {
                      TileMapLayerOverview(*tile_map);
                      ImGui::EndTabItem();
                  }
                  ImGui::EndTabBar();
              }
          } },
        { "SpriteTab", 0.0f,
          [&]() {
              ImGui::Text("???");
          } },
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    DrawContents(full_width, descs);
}

void AssetInspector::TileMapLayerOverview(TileMapAsset& p_tile_map) {
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

void AssetInspector::UpdateInternal(Scene*) {
    const auto& handle = m_editor.GetSelectedAsset();
    auto asset = handle.Get();
    if (!asset) {
        return;
    }

    switch (asset->type) {
        case AssetType::Sprite:
            InspectSprite(asset);
            break;
        case AssetType::TileMap:
            InspectTileMap(asset);
            break;
        default:
            break;
    }
}

}  // namespace my
