#include "TileMapEditor.h"

#include "TileMapLayerPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/algorithm/Graph.h"
#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/display/ICanvas.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/inspector/PropertyEditors.h"
#include "editor/panels/AssetInspector.h"
#include "editor/services/DocumentService.h"
#include "editor/services/EditorServices.h"
#include "editor/services/EditService.h"
#include "editor/tile_map/SetTileCommand.h"

// @TODO: remove
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "editor/utility/ImGuizmo.h"

namespace cave {

using namespace ::cave::math;

TileMapEditor::TileMapEditor(EditorState& editor,
                             DocId doc_id,
                             SceneId scene_id)
    : ViewTabBase(editor, doc_id, scene_id, ViewDimension::Dim2)
    , m_canvas(m_engine_services.canvas())
    , m_debug_id(MakeDebugId(this)) {

    m_tile_map_layer_panel = MakeOwner<TileMapLayerPanel>(m_sprite_selector);

    m_toolbar[0] = {
        "TileMapEditor.pencil",
        ICON_FA_PEN,
        "Pencil - paint individual tiles",
        [this]() { setPaintMode(GridPaintMode::Brush); },
        nullptr,
        [this]() { return m_paint_mode == GridPaintMode::Brush; },
    };
    m_toolbar[1] = {
        "TileMapEditor.line",
        ICON_FA_CHART_LINE,
        "Line - paint a straight line",
        [this]() { setPaintMode(GridPaintMode::Line); },
        nullptr,
        [this]() { return m_paint_mode == GridPaintMode::Line; },
    };
    m_toolbar[2] = {
        "TileMapEditor.rect",
        ICON_FA_SQUARE_PEN,
        "Rectangle - paint a filled rectangle",
        [this]() { setPaintMode(GridPaintMode::Rect); },
        nullptr,
        [this]() { return m_paint_mode == GridPaintMode::Rect; },
    };
    m_toolbar[3] = {
        "TileMapEditor.fill",
        ICON_FA_FILL,
        "Fill - replace a connected region",
        [this]() { setPaintMode(GridPaintMode::Fill); },
        nullptr,
        [this]() { return m_paint_mode == GridPaintMode::Fill; },
    };
    m_toolbar[4] = {
        "TileMapEditor.erase",
        ICON_FA_ERASER,
        "Eraser - remove painted tiles",
        [this]() { m_erasing = !m_erasing; },
        nullptr,
        [this]() { return m_erasing; },
    };
}

TileMapEditor::~TileMapEditor() = default;

void TileMapEditor::submitView() {
    ViewTabBase::submitView(false);
}

void TileMapEditor::drawGhostTiles(const TileSetAsset& tile_set) {
    const ImageAsset* image = tile_set.handle().get();
    if (!image) return;

    constexpr Vec4f kEraseColor{ 1.0f, 0.5f, 0.5f, 0.7f };

    auto selections = m_sprite_selector.GetSelections();

    bool selection_valid = false;
    Vec2f uv_min;
    Vec2f uv_max;
    if (!selections.empty()) {
        auto [x, y] = selections[0];
        uint32_t tile_id = y * tile_set.col() + x;
        const auto& frames = tile_set.frames();
        uv_min = frames[tile_id].min();
        uv_max = frames[tile_id].max();
        selection_valid = true;
    }

    for (const GridPaintCell& cell : m_paint_tool.preview()) {
        Vec2f min{ cell.coord.x, cell.coord.y };
        Vec2f max{ cell.coord.x + 1, cell.coord.y + 1 };

        if (m_erasing) {
            m_canvas.addBox2(min, max, kEraseColor);
            continue;
        }

        if (selection_valid) {
            m_canvas.addImage(image->gpu_texture.get(),
                              min, max,
                              Vec4f(Vec3f::One, 0.9f),
                              uv_min, uv_max);
        }
    }
}

void TileMapEditor::onInputEvents(const InputFrame& input) {
    m_camera_controller->update(input);

    IDocument* doc = m_editor_services.document().resolve(m_doc_id);
    DEV_ASSERT(doc);

    const TileMapAsset* tile_map = doc->handle<TileMapAsset>().get();
    if (!tile_map) return;

    const TileMapLayer* layer = m_tile_map_layer_panel->selectedLayer(*tile_map);
    if (!layer) return;

    GridPaintInput paint_input = buildInput(input);
    std::span<const GridPaintEvent> events = m_paint_tool.update(paint_input);
    for (const auto& event : events) {
        handlePaintEvent(event, *layer);
    }
}

void TileMapEditor::drawTileMap() {
    IDocument* doc = m_editor_services.document().resolve(m_doc_id);
    if (!DEV_VERIFY(doc)) {
        return;
    }

    const TileMapAsset* tile_map = doc->handle<TileMapAsset>().get();
    if (!tile_map) return;

    m_canvas.pushView(m_view_id);

    Vector<const TileMapLayer*> layers;
    for (const auto& layer : tile_map->layers()) {
        layers.push_back(&layer);
    }

    std::sort(layers.begin(), layers.end(), [](const TileMapLayer* a, const TileMapLayer* b) {
        return a->zIndex() < b->zIndex();
    });

    for (const auto& layer : layers) {
        if (!layer->visible()) {
            continue;
        }

        const TileSetAsset* tile_set = layer->handle().get();
        if (!tile_set) {
            continue;
        }

        const ImageAsset* image = tile_set->handle().get();
        if (!image) {
            continue;
        }

        const auto& frames = tile_set->frames();
        const auto& chunks = layer->chunks().chunks();
        for (const auto& [key, chunk] : chunks) {
            const int16_t offset_x = key.x * kTileChunkSize;
            const int16_t offset_y = key.y * kTileChunkSize;

            for (int16_t y = offset_y; y < offset_y + kTileChunkSize; ++y) {
                for (int16_t x = offset_x; x < offset_x + kTileChunkSize; ++x) {
                    const TileId& tile_id = chunk->at(x - offset_x, y - offset_y);
                    if ((int)frames.size() <= tile_id) {
                        continue;
                    }

                    const float s = tile_set->tileScale();
                    float x0 = s * x;
                    float y0 = s * y;
                    float x1 = s * (x + 1);
                    float y1 = s * (y + 1);

                    Vec2f uv_min = frames[tile_id].min();
                    Vec2f uv_max = frames[tile_id].max();

                    m_canvas.addImage(image->gpu_texture.get(),
                                      Vec2f(x0, y0),
                                      Vec2f(x1, y1),
                                      Vec4f::One,
                                      uv_min,
                                      uv_max);
                }
            }
        }
    }

    if (const TileMapLayer* layer = m_tile_map_layer_panel->selectedLayer(*tile_map)) {
        if (const TileSetAsset* tile_set = layer->handle().get()) {
            drawGhostTiles(*tile_set);
        }
    }

    m_canvas.popView();
}

void TileMapEditor::drawGizmo(const math::FloatRect& rect) {
    const Mat4f& proj_view = m_camera.projectionViewMatrix();

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(rect.x, rect.y, rect.w, rect.h);

    ImGuizmo::DrawGrid(proj_view, Mat4f(1.0f), 100.0f, ImGuizmo::GridPlane::XY);
}

void TileMapEditor::drawUIImpl() {
    ViewRecord* view = m_view_manager.resolve(m_view_id);
    DEV_ASSERT(view);

    updateRect(view->display_rect_os);

    drawTileMap();
    drawMainView(view->display_rect_os);
    drawGizmo(view->display_rect_os);

    submitView();
}

void TileMapEditor::drawToolbar() {
    DrawToolbar(m_toolbar);
}

void TileMapEditor::drawAssetInspector(IDocument& doc) {
    TileMapAsset* tile_map = doc.handle<TileMapAsset>().get();
    if (DEV_VERIFY(tile_map)) {
        DrawComponentCtx ctx = {
            .engine_services = m_engine_services,
            .editor_services = m_editor_services,
            .scene = nullptr,
            .entity = ecs::Entity::null(),
        };

        m_tile_map_layer_panel->draw(*tile_map, ctx);
    }
}

Option<TileCoord> TileMapEditor::pointToTile(math::Vec2f point_os) {
    if (!isVisible()) return None();

    const ViewRecord* view = m_view_manager.resolve(m_view_id);
    if (!view->display_rect_os.Contains(point_os.x, point_os.y)) {
        return None();
    }

    Vec2f ndc = view->screenToNDC(point_os);

    Mat4f pv_inv = glm::inverse(m_camera.projectionViewMatrix());

    Vec4f pos = pv_inv * Vec4f(ndc, 0.0f, 1.0f);
    pos /= pos.w;

    TileCoord index;
    index.x = static_cast<int16_t>(std::floor(pos.x));
    index.y = static_cast<int16_t>(std::floor(pos.y));
    return Some(index);
}

// ---- Paint Tool ----
GridPaintInput TileMapEditor::buildInput(const InputFrame& input) {
    GridPaintInput out{};

    Vec2f cursor = m_cursor.unwrap_or(Vec2f::Zero);

    for (const InputEvent& event : input.events) {
        Key key = static_cast<Key>(event.code);
        switch (event.type) {
            case InputEventType::ButtonDown:
                if (key == Key::LMB) {
                    out.left_down = true;
                    out.left_pressed = true;
                    event.consumed = true;
                    cursor = { event.x, event.y };
                }
                break;
            case InputEventType::ButtonUp:
                if (key == Key::LMB) {
                    out.left_down = false;
                    out.left_released = true;
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

    if (auto res = pointToTile(point_os)) {
        TileCoord coord = res.unwrap_unchecked();
        out.hover = { coord.x, coord.y };
        out.has_hover = true;
    }

    if (isHovered()) {
        m_cursor = Some(cursor);
    } else {
        m_cursor = None();
    }

    return out;
}

void TileMapEditor::setPaintMode(GridPaintMode mode) {
    m_paint_mode = mode;
    m_paint_tool.setMode(mode);
}

void TileMapEditor::handlePaintEvent(const GridPaintEvent& event,
                                     const TileMapLayer& layer) {
    switch (event.type) {
        case GridPaintEventType::Begin: {
            beginPaintCommand();
        } break;
        case GridPaintEventType::Apply:
            if (event.cells) {
                applyPaintCells(*event.cells, layer);
            }
            break;
        case GridPaintEventType::Fill: {
            if (DEV_VERIFY(event.cells && event.cells->size() == 1)) {
                applyFillCells(event.cells->at(0), layer);
            }
        } break;
        case GridPaintEventType::End: {
            finishPaintCommand();
        } break;
        case GridPaintEventType::Cancel: {
            cancelPaintCommand();
        } break;
    }
}

void TileMapEditor::beginPaintCommand() {
    DEV_ASSERT(m_pending_tile_changes.empty());
    m_pending_tile_changes.clear();
}

void TileMapEditor::finishPaintCommand() {
    if (m_pending_tile_changes.empty()) {
        return;
    }

    int layer_id = m_tile_map_layer_panel->selectedIndex().unwrap_or(-1);
    if (!DEV_VERIFY(layer_id >= 0)) {
        m_pending_tile_changes.clear();
        return;
    }

    auto composite = MakeOwner<SetTileCommand>(
        m_engine_services.sceneRegistry(), layer_id);

    for (const auto& [coord, change] : m_pending_tile_changes) {
        if (change.before == change.after) {
            continue;
        }

        composite->add(coord, change.before, change.after);
    }

    m_pending_tile_changes.clear();

    if (composite->empty()) {
        return;
    }

    m_editor_services.edit().submit(m_doc_id, std::move(composite));
}

void TileMapEditor::cancelPaintCommand() {
    m_pending_tile_changes.clear();
}

void TileMapEditor::applyPaintCells(std::span<const GridPaintCell> cells,
                                    const TileMapLayer& layer) {
    const auto selections = m_sprite_selector.GetSelections();
    uint32_t tile_id = std::numeric_limits<uint32_t>::max();

    const TileSetAsset* tile_set = layer.handle().get();
    DEV_ASSERT(tile_set);

    if (!m_erasing) {
        if (selections.empty()) {
            return;
        }

        const auto [x, y] = selections[0];
        if (x < 0 || y < 0) {
            return;
        }

        tile_id = static_cast<uint32_t>(y) * tile_set->col() +
                  static_cast<uint32_t>(x);

        if (tile_id >= tile_set->frames().size()) {
            return;
        }
    }

    for (const GridPaintCell& cell : cells) {
        if (cell.coord.x < std::numeric_limits<int16_t>::min() ||
            cell.coord.x > std::numeric_limits<int16_t>::max() ||
            cell.coord.y < std::numeric_limits<int16_t>::min() ||
            cell.coord.y > std::numeric_limits<int16_t>::max()) {
            continue;
        }

        const TileCoord coord{
            .x = static_cast<int16_t>(cell.coord.x),
            .y = static_cast<int16_t>(cell.coord.y),
        };

        Option<TileId> new_tile = None();

        if (!m_erasing) {
            new_tile = Some(static_cast<TileId>(tile_id));
        } else {
            new_tile = None();
        }

        auto pending_it = m_pending_tile_changes.find(coord);
        if (pending_it == m_pending_tile_changes.end()) {
            const Option<TileId> old_tile = layer.chunks().tileAt(coord);

            if (old_tile == new_tile) {
                continue;
            }

            m_pending_tile_changes.emplace(
                coord,
                PendingChange{
                    .before = old_tile,
                    .after = new_tile,
                });
        } else {
            // Preserve the original value from the beginning of the stroke,
            // but allow later brush passes to replace the final value.
            pending_it->second.after = new_tile;

            // If the stroke eventually restores the original value,
            // remove the no-op change entirely.
            if (pending_it->second.before == pending_it->second.after) {
                m_pending_tile_changes.erase(pending_it);
            }
        }
    }
}

// @TODO: better editor tools, make toolbars
void TileMapEditor::applyFillCells(GridPaintCell cell,
                                   const TileMapLayer& layer) {
    const int16_t x = static_cast<int16_t>(cell.coord.x);
    const int16_t y = static_cast<int16_t>(cell.coord.y);

    TileCoord world_coord{ x, y };
    TileChunkCoord chunk_coord = ToTileChunkCoord(world_coord);
    const auto& chunks = layer.chunks().chunks();
    auto it = chunks.find(chunk_coord);
    if (it == chunks.end()) { return; }

    const int16_t local_x = ToTileLocalX(world_coord);
    const int16_t local_y = ToTileLocalY(world_coord);

    std::span<const TileId> tile_data = it->second->tileData();

    auto tiles = FindConnectedTiles(tile_data,
                                    kTileChunkSize,
                                    kTileChunkSize,
                                    local_y * kTileChunkSize + local_x);

    if (tiles.empty()) {
        return;
    }

    Vector<GridPaintCell> paint_cells;
    paint_cells.reserve(tiles.size());

    for (int index : tiles) {
        GridCoord coord{
            index % kTileChunkSize + chunk_coord.x * kTileChunkSize,
            index / kTileChunkSize + chunk_coord.y * kTileChunkSize,
        };
        paint_cells.push_back({ coord });
    }

    applyPaintCells(paint_cells, layer);
    finishPaintCommand();
}

}  // namespace cave
