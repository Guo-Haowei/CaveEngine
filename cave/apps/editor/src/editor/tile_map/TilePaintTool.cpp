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
        m_erasing = context->tile.erasing;
        m_paint_tool.setMode(context->tile.paint_mode);
    }

    const TileMapLayerComponent* layer = getTileMapLayer(m_layer_id);
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

    constexpr Vec4f kEraseColor{ 1.0f, 0.5f, 0.5f, 0.7f };

    std::span<const uint32_t> selections;
    if (const auto* ctx = m_ctx.editor_services.sceneEdit().current()) {
        if (ctx->tile.valid()) {
            selections = ctx->tile.selected_tile;
        }
    }

    bool selection_valid = false;
    Vec2f uv_min{ 0, 0 };
    Vec2f uv_max{ 0, 0 };
    if (!selections.empty()) {
        uint32_t tile_id = selections[0];
        const auto& frames = tile_set.frames();
        uv_min = frames[tile_id].min();
        uv_max = frames[tile_id].max();
        selection_valid = true;
    }

    ICanvas& canvas = m_ctx.engine_services.canvas();
    canvas.pushView(m_ctx.view_id);

    for (const GridPaintCell& cell : m_paint_tool.preview()) {
        Vec2f min{ cell.coord.x, cell.coord.y };
        Vec2f max{ cell.coord.x + 1, cell.coord.y + 1 };

        if (m_erasing) {
            Draw2DOptions options = {
                .z_index = 0,
                .tint = kEraseColor,
            };
            canvas.addBox2(min, max, options);
            continue;
        }

        if (selection_valid) {
            ImageDrawOptions options{};
            options.tint = Vec4f(Vec3f::One, 0.9f);
            options.uv_min = uv_min;
            options.uv_max = uv_max;

            canvas.addImage(image->gpu_texture.get(), min, max, options);
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
                                     const TileMapLayerComponent& layer) {
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

void TilePaintTool::beginPaintCommand() {
    DEV_ASSERT(m_pending_tile_changes.empty());
    m_pending_tile_changes.clear();
}

void TilePaintTool::finishPaintCommand() {
    if (m_pending_tile_changes.empty()) {
        return;
    }

    auto composite = MakeOwner<SetTileCommand>(m_ctx.engine_services.sceneRegistry(),
                                               m_ctx.scene_id,
                                               m_layer_id);

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

    m_ctx.editor_services.edit().submit(m_ctx.doc_id, std::move(composite));
}

void TilePaintTool::cancelPaintCommand() {
    m_pending_tile_changes.clear();
}

void TilePaintTool::applyPaintCells(std::span<const GridPaintCell> cells,
                                    const TileMapLayerComponent& layer) {
    std::span<const uint32_t> selections;
    if (const auto* ctx = m_ctx.editor_services.sceneEdit().current()) {
        if (ctx->tile.valid()) {
            selections = ctx->tile.selected_tile;
        }
    }

    uint32_t tile_id = std::numeric_limits<uint32_t>::max();

    const TileSetAsset* tile_set = layer.tileSetHandle().get();
    DEV_ASSERT(tile_set);

    if (!m_erasing) {
        if (selections.empty()) {
            return;
        }

        tile_id  = selections[0];
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
void TilePaintTool::applyFillCells(GridPaintCell cell,
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
