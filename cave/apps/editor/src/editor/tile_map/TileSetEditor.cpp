#include "TileSetEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/display/ICanvas.h"

#include "editor/services/DocumentService.h"

// @TODO: move these to editor
#include "engine/private/runtime/ui/Inputs.h"
#include "engine/private/core/reflection/MetaEditor.h"

// @TODO: remove
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "editor/EditorState.h"

namespace cave {

using namespace ::cave::math;

namespace {

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

}  // namespace

TileSetEditor::TileSetEditor(EditorState& editor,
                             DocId doc_id,
                             SceneId scene_id)
    : ViewTabBase(editor, doc_id, scene_id, ViewDimension::Dim2)
    , m_debug_id(MakeDebugId(this)) {
}

TileSetEditor::~TileSetEditor() = default;

void TileSetEditor::onCreate() {
    ViewTabBase::onCreate();

    m_camera_controller->setMoveSpeed(CameraController2DEditor::kDefaultPanSpeed);
}

void TileSetEditor::onDestroy() {
    ViewTabBase::onDestroy();
}

void TileSetEditor::onInputEvents(const InputFrame& input) {
    if (!isHovered()) {
        return;
    }

    if (m_editor.isPlaying()) {
        return;
    }

    const KeyState& st = m_engine_services.inputService().keyState();
    if (!st.anyAltDown() && !st.anyCtrlDown() && !st.anyShiftDown()) {
        m_camera_controller->update(input);
    }

    Vec2f cursor = m_cursor.unwrap_or(Vec2f::Zero);

    for (const InputEvent& event : input.events) {
        Key key = static_cast<Key>(event.code);
        switch (event.type) {
            case InputEventType::ButtonDown:
                if (key == Key::LMB) {
                    // out.left_down = true;
                    // out.left_pressed = true;
                    event.consumed = true;
                    cursor = { event.x, event.y };
                }
                break;
            case InputEventType::ButtonUp:
                if (key == Key::LMB) {
                    // out.left_down = false;
                    // out.left_released = true;
                    event.consumed = true;
                }
                break;
            case InputEventType::MouseMove:
                cursor = { event.x, event.y };
                break;
            default:
                break;
        }
    }

    Vec2f point_os = cursor + m_engine_services.displayService().windowPos();

    m_atlas = None();
    if (auto assets = getAssets(); assets.tile_set) {
        if (auto res = worldPointToCell(point_os, *assets.tile_set)) {
            auto pos = res.unwrap_unchecked();
            m_atlas = Some(pos.x + pos.y * assets.tile_set->col());
        }
    }

    if (isHovered()) {
        m_cursor = Some(cursor);
    } else {
        m_cursor = None();
    }
}

void TileSetEditor::submitView() {
    ViewTabBase::submitView(false);
}

void TileSetEditor::drawUIImpl() {
    ViewRecord* view = m_view_manager.resolve(m_view_id);
    DEV_ASSERT(view);

    updateRect(view->display_rect_os);
    drawMainView(view->display_rect_os);
    drawTiles();

    submitView();
}

Option<math::Vec2i> TileSetEditor::worldPointToCell(math::Vec2f point_os,
                                                    const TileSetAsset& tile_set) const {
    const ViewRecord* view = m_engine_services.viewManager().resolve(m_view_id);
    if (!view) {
        return None();
    }

    auto res = ScreenPointToWorld2D(*view, m_camera.projectionViewMatrix(), point_os);
    if (!res) {
        return None();
    }

    const Vec2f point_ws = res.unwrap_unchecked();

    const Vec2f preview_size{
        static_cast<float>(tile_set.width()) / TileSetAsset::kDefaultCellSizePx,
        static_cast<float>(tile_set.height()) / TileSetAsset::kDefaultCellSizePx,
    };

    const Vec2f cell_size{
        preview_size.x / static_cast<float>(tile_set.col()),
        preview_size.y / static_cast<float>(tile_set.row()),
    };

    if (point_ws.x < 0.0f || point_ws.y < 0.0f || point_ws.x >= preview_size.x || point_ws.y >= preview_size.y) {
        return None();
    }

    const float x = std::floor(point_ws.x / cell_size.x);
    const float y = std::floor(point_ws.y / cell_size.y);
    return Some(Vec2i{ x, y });
}

void TileSetEditor::drawTiles() {
    ICanvas& canvas = m_engine_services.canvas();

    auto assets = getAssets();
    if (const TileSetAsset* tile_set = assets.tile_set; assets.image && tile_set) {
        canvas.pushView(m_view_id);

        const Vec2f image_size_px{ tile_set->width(), tile_set->height() };

        const Vec2f preview_size = image_size_px / TileSetAsset::kDefaultCellSizePx;

        canvas.addImage(assets.image->gpu_texture.get(), Vec2f::Zero, preview_size);

        const auto& frames = tile_set->frames();
        const uint32_t hovered_index = m_atlas.unwrap_or(std::numeric_limits<uint32_t>::max());

        for (const TileDefinition& definition : tile_set->getTileDefinitions()) {
            const uint32_t atlas_index = definition.id;

            if (atlas_index >= frames.size()) {
                continue;
            }

            const Box2& uv = frames[atlas_index];

            const Vec2f local_min = uv.min() * preview_size;
            const Vec2f local_max = uv.max() * preview_size;

            Draw2DOptions options;
            options.tint = atlas_index == hovered_index ? Vec4f(1.f, 1.f, 0.f, 0.7f)
                                                        : Vec4f(0.5f, 0.5f, 0.f, 0.7f);

            canvas.addBox2Frame(local_min, local_max, 0.4f, options);
        }
        canvas.popView();
    }
}

TileSetEditor::Assets TileSetEditor::getAssets() const {
    const IDocument* doc = m_editor_services.document().resolve(m_doc_id);
    if (!doc) return {};

    TileSetAsset* tile_set = doc->handle<TileSetAsset>().get();
    if (!tile_set) {
        return {};
    }

    ImageAsset* image = tile_set->handle().get();
    if (!image) {
        return {};
    }

    return { image, tile_set };
}

bool DrawTileDefinition(TileDefinition& definition) {
    bool changed = false;

    ImGui::PushID(static_cast<int>(definition.id));

    ImGui::Text("Tile %u", definition.id);
    ImGui::Separator();

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

    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();

        Option<int> pending_delete;

        for (int i = 0; i < static_cast<int>(definition.animation.size()); ++i) {
            TileFrame& frame = definition.animation[i];

            ImGui::PushID(i);

            ImGui::SeparatorText( std::format("Frame {}", i).c_str());

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

}  // namespace cave
