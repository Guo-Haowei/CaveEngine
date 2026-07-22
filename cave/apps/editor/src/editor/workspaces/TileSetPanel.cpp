#include "TileSetPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "editor/services/EditorServices.h"
#include "editor/services/IconCache.h"

namespace cave {

TileSetPanel::TileSetPanel(EngineServices& engine_services,
                           EditorServices& editor_services)
    : m_engine_services(engine_services)
    , m_editor_services(editor_services) {

    m_checkerboard_texture =
        m_editor_services.iconCache().getIconHandle(IconName::Checkerboard);
}

void TileSetPanel::drawTileSource() {
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
}

void TileSetPanel::drawPaint() {
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
}

void TileSetPanel::drawAtlas() {
    ImGui::TableSetColumnIndex(2);

    ImGui::BeginChild("##TileAtlasColumn");

    AtlasWidgetDesc desc;
    desc.id = "##TileAtlas";
    desc.texture = m_checkerboard_texture;
    desc.layout.image_size_px = { 192.0f, 64.0f };
    desc.layout.grid_size = { 3, 1 };
    desc.widget_size = { 0.0f, 0.0f };
    desc.show_toolbar = true;
    desc.show_checkerboard = true;
    m_atlas_widget.draw(desc, m_atlas_selection);

    ImGui::EndChild();
}

void TileSetPanel::draw() {
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_Resizable |
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

    drawTileSource();
    drawPaint();
    drawAtlas();

    ImGui::EndTable();
}

}  // namespace cave
