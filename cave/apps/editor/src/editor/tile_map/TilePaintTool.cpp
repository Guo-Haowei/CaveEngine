#include "TilePaintTool.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/algorithm/Graph.h"
#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/display/ICanvas.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/inspector/PropertyEditors.h"
#include "editor/windows/AssetWorkspace.h"
#include "editor/services/DocumentService.h"
#include "editor/services/EditorServices.h"
#include "editor/services/EditService.h"
#include "editor/services/SceneEditService.h"
#include "editor/tile_map/SetTileCommand.h"

// @TODO: remove
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "editor/utility/ImGuizmo.h"
#include "editor/windows/ViewTabBase.h"

namespace cave {

using namespace ::cave::math;

TilePaintTool::TilePaintTool(const SceneToolContext& ctx)
    : ISceneViewTool(ctx) {
}

TilePaintTool::~TilePaintTool() = default;

void TilePaintTool::onInputEvents(const InputFrame& input, const WindowState& state) {
    const auto* context = m_ctx.editor_services.sceneEdit().current();
    if (context && context->tile.valid()) {
        m_paint_tool.setMode(context->tile.paint_mode);
    }

    TileMapLayerComponent* layer = getTileMapLayer(m_layer_id);
    if (!layer) return;

    GridPaintInput paint_input = buildInput(input, state);
    std::span<const GridPaintEvent> events = m_paint_tool.update(paint_input);
    for (const auto& event : events) {
        handlePaintEvent(event, *layer);
    }
}

void TilePaintTool::draw(const math::FloatRect& rect) {
    if (const TileSetAsset* tile_set = getTileSet(m_layer_id)) {
        drawGhostTiles(*tile_set);
    }

    const Mat4f& proj_view = m_ctx.camera.projectionViewMatrix();

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(rect.x, rect.y, rect.w, rect.h);

    ImGuizmo::DrawGrid(proj_view, Mat4f(1.0f), 100.0f, ImGuizmo::GridPlane::XY);
}

void TilePaintTool::drawGhostTiles(const TileSetAsset& tile_set) {
    const ImageAsset* image = tile_set.handle().get();
    if (!image) return;

    std::span<const TileId> selections;
    if (const auto* ctx = m_ctx.editor_services.sceneEdit().current()) {
        if (ctx->tile.valid()) {
            selections = ctx->tile.selected_tile;
        }
    }

    const bool is_painting = m_paint_tool.currentAction() == GridPaintAction::Paint;

    bool selection_valid = false;
    Vec2f uv_min{ 0, 0 };
    Vec2f uv_max{ 0, 0 };
    if (!selections.empty()) {
        auto atlas_index = selections[0].value;
        const auto& frames = tile_set.frames();
        uv_min = frames[atlas_index].min();
        uv_max = frames[atlas_index].max();
        selection_valid = true;
    }

    ICanvas& canvas = m_ctx.engine_services.canvas();
    canvas.pushView(m_ctx.view_id);

    for (const GridPaintCell& cell : m_paint_tool.preview()) {
        Vec2f min{ cell.coord.x, cell.coord.y };
        Vec2f max{ cell.coord.x + 1, cell.coord.y + 1 };

        if (is_painting) {
            ImageDrawOptions options{};
            options.tint = Vec4f(Vec3f::One, 0.7f);
            options.uv_min = uv_min;
            options.uv_max = uv_max;
            canvas.addImage(image->gpu_texture.get(), min, max, options);
        } else {
            Draw2DOptions options = { .z_index = 0,
                                      .tint = { 1.0f, 0.5f, 0.5f, 0.7f } };
            canvas.addBox2(min, max, options);
        }
    }

    canvas.popView();
}

Option<TileCoord> TilePaintTool::pointToTile(math::Vec2f point_os) {
    const ViewRecord* view = m_ctx.engine_services.viewManager().resolve(m_ctx.view_id);
    if (!view) {
        return None();
    }

    auto res = ScreenPointToWorld2D(*view, m_ctx.camera.projectionViewMatrix(), point_os);
    if (res.is_none()) {
        return None();
    }

    TileCoord index;
    index.x = static_cast<int16_t>(std::floor(res.unwrap_unchecked().x));
    index.y = static_cast<int16_t>(std::floor(res.unwrap_unchecked().y));
    return Some(index);
}

// ---- Paint Tool ----
GridPaintInput TilePaintTool::buildInput(const InputFrame& input, const WindowState& state) {
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
                } else if (key == Key::RMB) {
                    out.right_down = true;
                    out.right_pressed = true;
                    event.consumed = true;
                    cursor = { event.x, event.y };
                }
                break;
            case InputEventType::ButtonUp:
                if (key == Key::LMB) {
                    out.left_down = false;
                    out.left_released = true;
                    event.consumed = true;
                } else if (key == Key::RMB) {
                    out.right_down = false;
                    out.right_released = true;
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

    Vec2f point_os = cursor + m_ctx.engine_services.displayService().windowPos();

    if (auto res = pointToTile(point_os)) {
        TileCoord coord = res.unwrap_unchecked();
        out.hover = { coord.x, coord.y };
        out.has_hover = true;
    }

    if (state.hovered) {
        m_cursor = Some(cursor);
    } else {
        m_cursor = None();
    }

    return out;
}

void TilePaintTool::handlePaintEvent(const GridPaintEvent& event,
                                     TileMapLayerComponent& layer) {
    switch (event.type) {
        case GridPaintEventType::Begin: {
            beginPaintCommand();
        } break;
        case GridPaintEventType::Apply:
            if (event.cells) {
                applyPaintCells(*event.cells, event.action, layer);
            }
            break;
        case GridPaintEventType::Fill: {
            CRASH_NOW_MSG("TODO: fix");
#if 0
            if (DEV_VERIFY(event.cells && event.cells->size() == 1)) {
                applyFillCells(event.cells->at(0), event.action, layer);
            }
#endif
        } break;
        case GridPaintEventType::End: {
            finishPaintCommand();
        } break;
        case GridPaintEventType::Cancel: {
            cancelPaintCommand();
        } break;
    }
}

void TilePaintTool::beginPaintCommand() {
    if (!DEV_VERIFY(m_active_command == nullptr)) {
        m_active_command.reset();
    }

    m_active_command = MakeOwner<SetTileCommand>(m_ctx.engine_services.sceneRegistry(),
                                                 m_ctx.scene_id,
                                                 m_layer_id);
}

void TilePaintTool::finishPaintCommand() {
    if (m_active_command == nullptr) {
        return;
    }
    if (m_active_command->empty()) {
        m_active_command.reset();
        return;
    }

    m_ctx.editor_services.edit().recordApplied(m_ctx.doc_id, std::move(m_active_command));
}

void TilePaintTool::cancelPaintCommand() {
    m_active_command.reset();
}

void TilePaintTool::applyPaintCells(std::span<const GridPaintCell> cells,
                                    GridPaintAction action,
                                    TileMapLayerComponent& layer) {
    DEV_ASSERT(m_active_command);

    std::span<const TileId> selections;
    if (const auto* ctx = m_ctx.editor_services.sceneEdit().current()) {
        if (ctx->tile.valid()) selections = ctx->tile.selected_tile;
    }

    const bool painting = action == GridPaintAction::Paint;
    const TileSetAsset* tile_set = layer.tileSetHandle().get();
    DEV_ASSERT(tile_set);

    const TileDefinition* definition = nullptr;
    if (painting && !selections.empty()) definition = tile_set->findTileDefinition(selections[0].value);
    if (painting && !definition) return;

    for (const GridPaintCell& cell : cells) {
        if (cell.coord.x < std::numeric_limits<int16_t>::min() || cell.coord.x > std::numeric_limits<int16_t>::max() ||
            cell.coord.y < std::numeric_limits<int16_t>::min() || cell.coord.y > std::numeric_limits<int16_t>::max()) {
            continue;
        }

        const TileCoord coord{ static_cast<int16_t>(cell.coord.x), static_cast<int16_t>(cell.coord.y) };
        const Option<TileCell> before = layer.chunks().cellAt(coord);
        const Option<TileCell> after = painting
                                           ? Some(TileCell{ TileId(static_cast<uint16_t>(definition->id)), definition->terrain_id })
                                           : Option<TileCell>(None());

        if (before == after) continue;

        m_active_command->record(coord, before, after);

        if (after.is_some()) {
            layer.setCell(coord, after.unwrap_unchecked());
        } else {
            layer.removeCell(coord);
        }
    }
}

// @TODO: better editor tools, make toolbars
#if 0
void TilePaintTool::applyFillCells(GridPaintCell cell,
                                   GridPaintAction action,
                                   const TileMapLayerComponent& layer) {
    const int16_t x = static_cast<int16_t>(cell.coord.x);
    const int16_t y = static_cast<int16_t>(cell.coord.y);

    TileCoord world_coord{ x, y };
    TileChunkCoord chunk_coord = ToTileChunkCoord(world_coord);
    const auto& chunks = layer.chunks().chunks();
    auto it = chunks.find(chunk_coord);
    if (it == chunks.end()) { return; }

    const int16_t local_x = ToTileLocalX(world_coord);
    const int16_t local_y = ToTileLocalY(world_coord);

    std::span<const TileCell> tile_data = it->second->tileData();

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

    applyPaintCells(paint_cells, action, layer);
    finishPaintCommand();
}
#endif

TileMapLayerComponent* TilePaintTool::getTileMapLayer(ecs::Entity entity) {
    if (Scene* scene = getResolvedScene()) {
        return scene->component<TileMapLayerComponent>(entity);
    }
    return nullptr;
}

TileSetAsset* TilePaintTool::getTileSet(ecs::Entity entity) {
    if (const TileMapLayerComponent* tile_layer = getTileMapLayer(entity)) {
        return tile_layer->tileSetHandle().get();
    }
    return nullptr;
}

}  // namespace cave
