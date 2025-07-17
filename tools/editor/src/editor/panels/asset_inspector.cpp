#include "asset_inspector.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/assets/assets.h"
#include "engine/assets/sprite_asset.h"
#include "engine/runtime/asset_registry.h"
#include "editor/editor_layer.h"
#include "editor/widget.h"
#include "editor/viewer/tile_map_editor.h"

namespace cave {

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
        {
            "ImageSource",
            360.0f,
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
            },
        },
        {
            "SpriteEditor",
            360.0f,
            [&]() { EditSprite(*sprite); },
        },
        {
            "TileSetPanel",
            0.0f,
            [&]() { TilePaint(*sprite); },
        },
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    DrawContents(full_width, descs);
}

void AssetInspector::InspectTileMap(IAsset* p_asset) {
    auto tile_map = dynamic_cast<TileMapAsset*>(p_asset);
    if (!tile_map) {
        return;
    }

    SpriteAsset* sprite = nullptr;
    if (auto tool = dynamic_cast<TileMapEditor*>(m_editor.GetActiveTool()); tool) {
        if (auto layer = tool->GetActiveLayer(); layer) {
            sprite = layer->GetSpriteHandle().Get();
        }
    }

    std::vector<AssetChildPanel> descs = {
        {
            "LayerOverview",
            360,
            [&]() {
                if (ImGui::BeginTabBar("##MyTabs1")) {
                    if (ImGui::BeginTabItem("Layer")) {
                        TileMapLayerOverview(*tile_map);
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
                if (sprite) {
                    EditSprite(*sprite);
                }
            },
        },
        {
            "PaintTab",
            0,
            [&]() {
                if (sprite) {
                    TilePaint(*sprite);
                }
            },
        }
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    DrawContents(full_width, descs);
}

void AssetInspector::TileMapLayerOverview(TileMapAsset& p_tile_map) {
    if (ImGui::Button(ICON_FA_SQUARE_PLUS " Add Layer")) {
        // p_tile_map.AddLayer("untitled layer");
    }
    ImGui::Separator();

    auto checkerboard = m_checkerboard_handle.Get();
    DEV_ASSERT(checkerboard);

    auto tool = dynamic_cast<TileMapEditor*>(m_editor.GetActiveTool());
    DEV_ASSERT(tool);

    const int current_layer = tool->GetActiveLayerIndex();

    for (int layer_id = 0; layer_id < 1; ++layer_id) {
        TileMapAsset& layer = p_tile_map;
        const bool is_layer_selected = current_layer == layer_id;

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

        if (DrawInputText("layer", layer.GetName())) {
            // @TODO: notify dirty
        }

        ImGui::SameLine();

        const bool is_visible = layer.IsVisible();
        const char* label = is_visible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
        if (ImGui::Button(label)) {
            layer.SetVisible(!is_visible);
        }

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_TRASH_CAN)) {
            LOG_WARN("TODO: DELETE");
        }

        // next line

        ImVec2 region_size(128, 128);
        ImVec2 image_size = region_size;

        uint64_t image_handle = 0;

        if (auto sprite = layer.GetSpriteHandle().Get(); sprite) {
            if (auto image = sprite->GetHandle().Get(); image) {
                image_handle = image->gpu_texture ? image->gpu_texture->GetHandle() : 0;
                image_size = ImVec2(static_cast<float>(image->width),
                                    static_cast<float>(image->height));
            }
        }
        if (image_handle == 0) {
            image_handle = checkerboard->gpu_texture->GetHandle();
        }

        CenteredImage(image_handle, image_size, region_size);

        if (ImGui::IsItemClicked()) {
            tool->SetActiveLayer(layer_id);
        }

        DragDropTarget(AssetType::Sprite, [&](AssetHandle& p_handle) {
            DEV_ASSERT(p_handle.GetMeta()->type == AssetType::Sprite);
            layer.SetSpriteGuid(p_handle.GetGuid());
        });

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

}  // namespace cave
