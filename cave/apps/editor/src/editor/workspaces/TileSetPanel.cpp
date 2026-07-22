#include "TileSetPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/services/EditorServices.h"
#include "editor/services/IconCache.h"
#include "editor/services/SceneEditService.h"
#include "editor/widgets/Image.h"

// @TODO: refactor
#include "engine/private/runtime/ui/Inputs.h"
#include "engine/private/core/reflection/MetaEditor.h"

namespace cave {

using namespace ::cave::math;

namespace {

#if 0
void DrawPhysicsTab(TileSetAsset& tile_set, SpriteSelector& sprite_selector) {
    int index = -1;
    if (auto selected = sprite_selector.GetSelections(); !selected.empty()) {
        auto [x, y] = selected.front();
        index = tile_set.col() * y + x;
    }

    ToolbarButtonDesc add_square_button_desc = {
        "TileSetEditor.physics.box",
        ICON_FA_SQUARE " Box", "Add box collider",
        [&]() {
            // if (index >= 0 && tile_set.addBoxCollider(index)) {
            //     LOG_OK("Box collider added for {}", index);
            // } else {
            //     LOG_ERROR("Failed to add box collider for {}", index);
            // }
        }
    };

    ToolbarButtonDesc add_polygon_button_desc = {
        "TileSetEditor.physics.polygon",
        ICON_FA_DRAW_POLYGON " Polygon", "Add polygon collider",
        [&]() {
            LOG_WARN("Not implemented");
        }
    };

    ToolbarButtonDesc add_circle_button_desc = {
        "TileSetEditor.circle.polygon",
        ICON_FA_CIRCLE " Circle", "Add circle collider",
        [&]() {
            LOG_WARN("Not implemented");
        }
    };

    Vector<const ToolbarButtonDesc*> tool_bar = {
        &add_square_button_desc,
        &add_polygon_button_desc,
        &add_circle_button_desc,
    };

    DrawToolbar(tool_bar);
    ImGui::Separator();
}
#endif

bool EditSprite(int* colomn, int* row) {
    bool dirty = false;
    if (ImGui::BeginTabBar("TileSetModes")) {
        if (ImGui::BeginTabItem("Setup")) {
            if (ImGui::InputInt("column", colomn)) {
                *colomn = std::max(*colomn, 1);
                dirty = true;
            }
            if (ImGui::InputInt("row", row)) {
                *row = std::max(*row, 1);
                dirty = true;
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    return dirty;
}

bool DrawTileDefinition(TileDefinition& definition) {
    bool changed = false;

    ImGui::PushID(static_cast<int>(definition.id));

    ImGui::Text("Tile %u", definition.id);
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Physics", 0)) {
        ImGui::Indent();

        DrawEnumDropDown<CollisionType>("Type", definition.collision, ui::kDefaultColumnWidth);

        if (definition.collision != CollisionType::None) {
            ImGui::Spacing();

            Vec2f min = definition.collision_shape.min();
            Vec2f max = definition.collision_shape.max();

            if (ui::Float2("Min", min)) {
                max = math::max(max, min);
                definition.collision_shape = Box2{ min, max };
                changed = true;
            }

            if (ui::Float2("Max", max, 1.0f)) {
                min = math::min(min, max);
                definition.collision_shape = Box2{ min, max };
                changed = true;
            }

            // Optional normalization to tile-local coordinates.
            Vec2f clamped_min = math::clamp(definition.collision_shape.min(), Vec2f::Zero, Vec2f::One);
            Vec2f clamped_max = math::clamp(definition.collision_shape.max(), Vec2f::Zero, Vec2f::One);

            if (clamped_min != definition.collision_shape.min() ||
                clamped_max != definition.collision_shape.max()) {
                definition.collision_shape = Box2{ clamped_min, clamped_max };
                changed = true;
            }

            ImGui::Spacing();

            if (ui::DrawBitMask32("Mask", definition.mask)) {
                changed = true;
            }
        }
        ImGui::Unindent();
    }

    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Animation", 0)) {
        ImGui::Indent();

        Option<int> pending_delete;

        for (int i = 0; i < static_cast<int>(definition.animation.size()); ++i) {
            TileFrame& frame = definition.animation[i];

            ImGui::PushID(i);

            ImGui::SeparatorText(std::format("Frame {}", i).c_str());

            int atlas_index = static_cast<int>(frame.atlas_index);

            if (ui::InputInt("Atlas Index", atlas_index)) {
                frame.atlas_index = static_cast<uint32_t>(std::max(atlas_index, 0));

                changed = true;
            }

            if (ui::DragFloat("Duration", frame.duration, 0.01f, 0.01f, 10.0f)) {
                changed = true;
            }

            if (ImGui::Button(ICON_FA_TRASH_CAN " Remove")) {
                pending_delete = Some(i);
            }

            ImGui::PopID();
        }

        if (pending_delete) {
            definition.animation.erase(definition.animation.begin() + pending_delete.unwrap_unchecked());
            changed = true;
        }

        if (ImGui::Button(ICON_FA_PLUS " Add Frame")) {
            TileFrame frame;

            if (!definition.animation.empty()) {
                frame.atlas_index = definition.animation.back().atlas_index;
                frame.duration = definition.animation.back().duration;
            } else {
                frame.atlas_index = definition.id;
            }

            definition.animation.push_back(frame);
            changed = true;
        }

        ImGui::Unindent();
    }

    ImGui::PopID();
    return changed;
}

}  // namespace

TileSetPanel::TileSetPanel(EngineServices& engine_services,
                           EditorServices& editor_services)
    : m_engine_services(engine_services)
    , m_editor_services(editor_services) {

    m_checkerboard = editor_services.iconCache().getIconHandle(IconName::Checkerboard);
}

void TileSetPanel::drawTileSource(TileSetAsset* tile_set) {
    unused(tile_set);

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

    ImGui::EndChild();

    const float button_size = ImGui::GetFrameHeight();

    ImGui::Button(ICON_FA_TRASH_CAN, ImVec2{ button_size, 0.0f });

    ImGui::SameLine();

    ImGui::Button(ICON_FA_PLUS, ImVec2{ button_size, 0.0f });

    ImGui::SameLine();

    ImGui::Button(ICON_FA_ELLIPSIS_VERTICAL, ImVec2{ button_size, 0.0f });

    ImGui::EndChild();
}

void TileSetPanel::drawTileProperties(TileSetAsset* tile_set) {
    ImGui::TableSetColumnIndex(1);

    ImGui::BeginChild("##TileToolsColumn", ImVec2{ 0.0f, 0.0f });

    auto mode_button = [&](Property value, const char* label) {
        const bool active = m_mode == value;
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        }

        if (ImGui::Button(label)) {
            m_mode = value;
        }

        if (active) {
            ImGui::PopStyleColor();
        }
    };

    mode_button(Property::Setup, ICON_FA_SCREWDRIVER_WRENCH " Setup");
    ImGui::SameLine();
    mode_button(Property::SelectedTile, ICON_FA_ARROW_POINTER " Select");
    ImGui::SameLine();
    mode_button(Property::Paint, ICON_FA_PAINTBRUSH " Paint");
    ImGui::Separator();

    switch (m_mode) {
        case Property::Setup: {
            ImGui::TextUnformatted("Setup Properties");
            if (tile_set) {
                int col = tile_set->col();
                int row = tile_set->row();

                if (EditSprite(&col, &row)) {
                    tile_set->col(col);
                    tile_set->row(row);
                }
            }
        } break;
        case Property::SelectedTile: {
            ImGui::TextUnformatted("Selected Tile Properties");
            if (tile_set) {
                for (TileDefinition& definition : tile_set->getTileDefinitionsMut()) {
                    DrawTileDefinition(definition);
                }
            }
        } break;
        case Property::Paint: {
            ImGui::TextUnformatted("Paint Properties");
            ImGui::Spacing();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::Combo("##PaintProperty", &m_paint_property, "Physics\0Terrain\0Animation\0");
        } break;
    }

    ImGui::EndChild();
}

void TileSetPanel::drawAtlas(TileSetAsset* tile_set, ImageAsset* image) {
    ImGui::TableSetColumnIndex(2);

    ImGui::BeginChild("##TileAtlasColumn");

    AtlasWidgetDesc desc;
    desc.id = "##TileAtlas";
    desc.texture = 0;
    desc.layout.image_size_px = { 64.0f, 64.0f };
    desc.layout.grid_size = { 1, 1 };
    desc.widget_size = { 0.0f, 0.0f };
    desc.show_toolbar = true;
    desc.show_checkerboard = true;

    if (tile_set) {
        desc.layout.grid_size = Vec2i(tile_set->col(), tile_set->row());
    }

    if (image && image->gpu_texture) {
        desc.texture = image->gpu_texture->GetHandle();
        desc.layout.image_size_px = Vec2f(image->width, image->height);
    }

    m_atlas_widget.draw(desc, m_atlas_selection);

    ImGui::EndChild();
}

void TileSetPanel::draw(SceneEditContext* context) {
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_Resizable |
                                      ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_SizingStretchProp;

    if (!ImGui::BeginTable("##TileSetEditorLayout", 3,
                           flags,
                           ImGui::GetContentRegionAvail())) {
        return;
    }

    ImageAsset* image = nullptr;
    TileSetAsset* tile_set = nullptr;
    if (context && context->tile.layer_entity.valid()) {
        tile_set = context->tile.tile_set.get();
        image = context->tile.image.get();
    }

    ImGui::TableSetupColumn("##Sources", ImGuiTableColumnFlags_WidthFixed, 280.0f);
    ImGui::TableSetupColumn("##Tools", ImGuiTableColumnFlags_WidthFixed, 360.0f);
    ImGui::TableSetupColumn("##Atlas", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableNextRow();

    drawTileSource(tile_set);
    drawTileProperties(tile_set);
    drawAtlas(tile_set, image);

    ImGui::EndTable();
}

#if 0
void TileSetEditor::drawAssetInspector(IDocument&) {
    auto assets = getAssets();
    if (!DEV_VERIFY(assets.image && assets.tile_set)) {
        return;
    }

    TileSetAsset& tile_set = *assets.tile_set;
    {
        int column = tile_set.col();
        int row = tile_set.row();
        if (m_sprite_selector.EditSprite(&column, &row)) {
            tile_set.col(column);
            tile_set.row(row);
        }
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("TileSetPhysics")) {
        if (ImGui::BeginTabItem("Physics Layer")) {
            DrawPhysicsTab(tile_set, m_sprite_selector);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();

    for (TileDefinition& definition : tile_set.getTileDefinitionsMut()) {
        DrawTileDefinition(definition);
    }
}
#endif

}  // namespace cave
