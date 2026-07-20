#include "TileMapLayerPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/inspector/PropertyEditors.h"
#include "editor/services/DragDropService.h"
#include "editor/services/EditorServices.h"
#include "editor/services/IconCache.h"
#include "editor/widgets/Image.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

using namespace ::cave::math;

void TileMapLayerPanel::draw(TileMapAsset& tile_map, DrawObjectCtx& ctx) {
    if (ImGui::BeginTabBar("##MyTabs1")) {
        if (ImGui::BeginTabItem("Layer")) {
            drawToolbar(tile_map);
            drawLayers(tile_map, ctx);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();

    if (const TileMapLayer* layer = selectedLayer(tile_map)) {
        if (TileSetAsset* tile_set = layer->handle().get()) {
            auto handle = tile_set->handle();
            const int column = tile_set->col();
            const int row = tile_set->row();
            if (auto image = handle.get(); image) {
                m_sprite_selector.SelectSprite(*image, &column, &row);
            }
        }
    }
}

void TileMapLayerPanel::drawToolbar(TileMapAsset& tile_map) {
    const float button_width = ImGui::GetFrameHeight();
    auto& layers = tile_map.layers();

    ImGui::BeginGroup();

    if (ImGui::Button(ICON_FA_PLUS, ImVec2{ button_width, 0.0f })) {
        layers.emplace_back();
        const int new_index = static_cast<int>(layers.size()) - 1;
        layers.back().name() = std::format("Layer {}", layers.size());
        m_selected = Some(new_index);
        // @TODO: mark dirty
    }

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("Add a new layer");
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(!m_selected);

    if (ImGui::Button(ICON_FA_TRASH_CAN, ImVec2{ button_width, 0.0f })) {
        if (m_selected) {
            const int deleted = m_selected.unwrap_unchecked();
            layers.erase(layers.begin() + deleted);
            m_selected = None();
            // @TODO: mark dirty
        }
    }

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip(m_selected ? "Delete selected layer" : "Select a layer first");
    }

    ImGui::EndDisabled();

    ImGui::EndGroup();
}

void TileMapLayerPanel::drawLayers(TileMapAsset& tile_map, DrawObjectCtx& ctx) {
    const IconCache& icons = ctx.editor_services.iconCache();

    auto notify_changed = [&]() {
        // tile_map.incRevision();
    };

    auto& layers = tile_map.layers();

    constexpr float kLayerCardHeight = 230.0f;

    ImGui::BeginChild("##LayerList",
                      ImVec2{ 0.0f, kLayerCardHeight * (float)layers.size() },
                      ImGuiChildFlags_Borders);

    for (int layer_id = 0; layer_id < static_cast<int>(layers.size()); ++layer_id) {
        TileMapLayer& layer = layers[layer_id];

        const bool selected =
            m_selected.unwrap_or(-1) == layer_id;

        ImGui::PushID(layer_id);

        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_Header));
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 2.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 8.0f, 8.0f });

        const ImGuiChildFlags child_flags = ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY;

        ImGui::BeginChild("##LayerCard",
                          ImVec2{ 0.0f, 0.0f },
                          child_flags,
                          ImGuiWindowFlags_NoScrollbar);

        const bool card_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

        if (ui::TextBox("Name", layer.name())) {
            m_selected = Some(layer_id);
            notify_changed();
        }

        ImGui::SameLine();

        const bool visible = layer.visible();
        const char* visibility_icon = visible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;

        if (ImGui::Button(visibility_icon)) {
            layer.setVisible(!visible);
            m_selected = Some(layer_id);
            notify_changed();
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(visible ? "Hide layer" : "Show layer");
        }

        if (Guid tile_set_guid = layer.tileSetGuid();
            DrawAsset(ctx, "Tile Set", tile_set_guid)) {
            layer.setTileSetGuid(tile_set_guid);
            m_selected = Some(layer_id);
            notify_changed();
        }

        int z_index = layer.zIndex();
        if (ui::InputInt("z_index", z_index)) {
            layer.setZIndex(z_index);
            m_selected = Some(layer_id);
            notify_changed();
        }

        const ImageAsset* image = nullptr;
        if (const TileSetAsset* tile_set = layer.handle().get()) {
            image = tile_set->handle().get();
        }

        constexpr Vec2f preview_size{ 96.0f, 96.0f };

        ui::CenteredImage(image, preview_size, icons.getIconHandle(IconName::Checkerboard));

        if (ImGui::IsItemClicked()) {
            m_selected = Some(layer_id);
        }

        if (card_hovered &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
            !ImGui::IsAnyItemHovered()) {
            m_selected = Some(layer_id);
        }

        ImGui::EndChild();

        ImGui::PopStyleVar(2);

        if (selected) {
            ImGui::PopStyleColor();
        }

        ImGui::PopID();

        ImGui::Spacing();
    }

    ImGui::EndChild();
}

const TileMapLayer* TileMapLayerPanel::selectedLayer(const TileMapAsset& tile_map) {
    if (m_selected.is_none()) {
        return nullptr;
    }

    const int idx = m_selected.unwrap_unchecked();
    auto layers = tile_map.layers();
    if (idx < 0 || idx > layers.size()) {
        return nullptr;
    }

    return &layers[idx];
}

}  // namespace cave
