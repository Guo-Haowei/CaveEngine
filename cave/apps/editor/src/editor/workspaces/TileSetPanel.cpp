#include "TileSetPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

namespace cave {

namespace {

#if 0
void DrawImageCanvasTest(
    ImageCanvas& canvas,
    ImTextureID checkerboard_handle) {

    constexpr Vec2f kTileSizePx{
        32.0f,
        32.0f,
    };

    constexpr Vec2f kGridSize{
        1.0f,
        1.0f,
    };

    constexpr Vec2f kImageSizePx{
        kTileSizePx.x * kGridSize.x,
        kTileSizePx.y * kGridSize.y,
    };

    ImageCanvasDesc desc;
    desc.id = "##ImageCanvasTest";
    desc.texture = checkerboard_handle;
    desc.image_size_px = kImageSizePx;
    desc.widget_size = {
        0.0f,
        320.0f,
    };

    desc.min_zoom = 0.25f;
    desc.max_zoom = 16.0f;
    desc.zoom_step = 2.0f;

    desc.show_toolbar = true;
    desc.allow_mouse_wheel_zoom = true;
    desc.show_checkerboard = false;

    const ImageCanvasResult result =
        canvas.draw(desc);

    if (result.mouse_image_px) {
        const Vec2f point = result.mouse_image_px.unwrap_unchecked();

        const int tile_x = static_cast<int>(point.x / kTileSizePx.x);
        const int tile_y = static_cast<int>(point.y / kTileSizePx.y);

        ImGui::Text( "Pixel: %.1f, %.1f    Tile: %d, %d    Zoom: %.0f%%",
            point.x,
            point.y,
            tile_x,
            tile_y,
            canvas.zoom() * 100.0f);
    } else {
        ImGui::Text(
            "Pixel: --, --    Zoom: %.0f%%",
            canvas.zoom() * 100.0f);
    }
}
#endif

}  // namespace
void TileSetPanel::draw() {
    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_SizingStretchProp;

    if (!ImGui::BeginTable("##TileSetEditorLayout", 3,
                           flags,
                           ImGui::GetContentRegionAvail())) {
        return;
    }

    ImGui::TableSetupColumn("##Sources", ImGuiTableColumnFlags_WidthFixed, 280.0f);

    ImGui::TableSetupColumn("##Tools", ImGuiTableColumnFlags_WidthFixed, 360.0f);

    ImGui::TableSetupColumn("##Atlas", ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableNextRow();

    // ---------------------------------------------------------------------
    // Column 1: Tile Sources
    // ---------------------------------------------------------------------

    ImGui::TableSetColumnIndex(0);

    ImGui::BeginChild("##TileSourcesColumn", ImVec2{ 0.0f, 0.0f });

    if (ImGui::BeginTabBar("##TileSourceTabs")) {
        if (ImGui::BeginTabItem("Tile Sources")) {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Patterns")) {
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::BeginChild("##TileSourceList",
                      ImVec2{ 0.0f, -ImGui::GetFrameHeightWithSpacing() },
                      ImGuiChildFlags_Borders);

    // Placeholder source entry.
    ImGui::Selectable("background.tileset",
                      true,
                      ImGuiSelectableFlags_None,
                      ImVec2{ 0.0f, 72.0f });

    ImGui::EndChild();

    const float button_size = ImGui::GetFrameHeight();

    ImGui::Button(ICON_FA_TRASH_CAN, ImVec2{ button_size, 0.0f });

    ImGui::SameLine();

    ImGui::Button(ICON_FA_PLUS, ImVec2{ button_size, 0.0f });

    ImGui::SameLine();

    ImGui::Button(ICON_FA_ELLIPSIS_VERTICAL, ImVec2{ button_size, 0.0f });

    ImGui::EndChild();

    // ---------------------------------------------------------------------
    // Column 2: Setup / Select / Paint + properties
    // ---------------------------------------------------------------------

    ImGui::TableSetColumnIndex(1);

    ImGui::BeginChild(
        "##TileToolsColumn",
        ImVec2{ 0.0f, 0.0f });

    static int mode = 0;

    auto mode_button = [&](int value, const char* label) {
        const bool active = mode == value;

        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        }

        if (ImGui::Button(label)) {
            mode = value;
        }

        if (active) {
            ImGui::PopStyleColor();
        }
    };

    mode_button(0, ICON_FA_SCREWDRIVER_WRENCH " Setup");

    ImGui::SameLine();

    mode_button(1, ICON_FA_ARROW_POINTER " Select");

    ImGui::SameLine();

    mode_button(2, ICON_FA_PAINTBRUSH " Paint");

    ImGui::Separator();

    switch (mode) {
        case 0:
            ImGui::TextUnformatted("Setup Properties");
            break;

        case 1:
            ImGui::TextUnformatted("Selected Tile Properties");
            break;

        case 2:
            ImGui::TextUnformatted("Paint Properties");

            ImGui::Spacing();

            static int paint_property = 0;

            ImGui::SetNextItemWidth(-1.0f);
            ImGui::Combo(
                "##PaintProperty",
                &paint_property,
                "Physics\0Terrain\0Animation\0");

            break;
    }

    ImGui::EndChild();

    // ---------------------------------------------------------------------
    // Column 3: Atlas
    // ---------------------------------------------------------------------

    ImGui::TableSetColumnIndex(2);

    ImGui::BeginChild(
        "##TileAtlasColumn",
        ImVec2{ 0.0f, 0.0f });

    AtlasWidgetDesc desc;
    desc.id = "##TileAtlas";
    desc.texture = m_checkerboard_texture;

    desc.layout.image_size_px = { 192.0f, 64.0f };

    desc.layout.grid_size = { 3, 1 };

    desc.widget_size = { 0.0f, 0.0f };

    desc.show_toolbar = true;
    desc.show_checkerboard = false;

    m_atlas_widget.draw(desc, m_atlas_selection);

    ImGui::EndChild();

    ImGui::EndTable();
}

}  // namespace cave
