#include "SceneTab.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/tile_map/TileMapLayerComponent.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/scene_view/SceneSelectTool.h"
#include "editor/services/EditService.h"
#include "editor/services/EditorServices.h"
#include "editor/services/PickingService.h"
#include "editor/services/SelectionService.h"
#include "editor/tile_map/TilePaintTool.h"

// @TODO: refactor
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/view/ViewManager.h"

#include "editor/EditorState.h"

namespace cave {

using namespace ::cave::math;
using ecs::Entity;

SceneTab::SceneTab(EditorState& editor,
                   DocId doc_id,
                   SceneId scene_id,
                   ViewDimension dim)
    : ViewTabBase(editor, doc_id, scene_id, dim)
    , m_editor(editor)
    , m_debug_id(MakeDebugId(this)) {

    m_play_button = {
        "SceneTab.play",
        ICON_FA_PLAY,
        "Run Project",
        [this]() {
            m_editor.requestModeSwitch();
        },
        [this]() {
            return !m_editor.isPlaying();
        },
    };
    m_pause_button = {
        "SceneTab.pause",
        ICON_FA_PAUSE,
        "Pause Project",
        [this]() {
            m_editor.requestModeSwitch();
        },
        [this]() {
            return m_editor.isPlaying();
        },
    };
}

// @TODO: game view tab
void SceneTab::submitView() {
    ViewTabBase::submitView(true);
}

void SceneTab::onCreate() {
    ViewTabBase::onCreate();

    SceneToolContext ctx = {
        .engine_services = m_engine_services,
        .editor_services = m_editor_services,
        .camera = m_camera,
        .view_id = m_view_id,
        .scene_id = m_preview_scene_id,
        .doc_id = m_doc_id,
    };

    m_scene_tools[std::to_underlying(SceneViewToolType::Select)] =
        MakeOwner<SceneSelectTool>(ctx);
    m_scene_tools[std::to_underlying(SceneViewToolType::TilePaint)] =
        MakeOwner<TilePaintTool>(ctx);
    m_tile_paint_tool = static_cast<TilePaintTool*>(m_scene_tools[std::to_underlying(SceneViewToolType::TilePaint)].get());

    m_editor_services.picking().addConsumer(this);

    updateSceneEditContext();
}

void SceneTab::onDestroy() {
    ViewTabBase::onDestroy();

    m_editor_services.picking().removeConsumer(this);
    m_editor_services.sceneEdit().deactivate(&m_edit_context);
}

void SceneTab::onActivated() {
    m_editor_services.sceneEdit().activate(&m_edit_context);
}

void SceneTab::onDeactivated() {
    m_editor_services.sceneEdit().deactivate(&m_edit_context);
}

void SceneTab::updateSceneEditContext() {
    m_edit_context.doc_id = m_doc_id;
    m_edit_context.scene_id = m_preview_scene_id;

    auto selection = m_editor_services.selection().primary(m_doc_id);
    auto entity = selection.entity;
    m_edit_context.selected_entity = entity;

    const Scene* scene = m_engine_services.sceneRegistry().resolve(m_preview_scene_id);
    if (scene) {
        auto& tile = m_edit_context.tile;
        if (const auto* layer = scene->component<TileMapLayerComponent>(entity)) {
            tile.layer_entity = entity;
            tile.tile_set = layer->tileSetHandle();
            if (const auto* tile_set = tile.tile_set.get()) {
                tile.image = tile_set->handle();
            }
        } else {
            tile.layer_entity = Entity::null();
            tile.tile_set.invalidate();
            tile.image.invalidate();
        }
    }
}

Option<PickData> SceneTab::getPickData(const Vec2f& point_os) {
    if (!isVisible()) return None();

    return activeTool()->getPickData(point_os);
}

void SceneTab::drawToolbar() {
    std::array<const ToolbarButtonDesc*, 2> descs = {
        &m_play_button,
        &m_pause_button,
    };

    DrawToolbar(descs);
}

void SceneTab::onInputEvents(const InputFrame& input) {
    if (!isHovered()) {
        return;
    }

    if (m_editor.isPlaying()) {
        return;
    }

    activeTool()->onInputEvents(input, windowState());

    const KeyState& st = m_engine_services.inputService().keyState();
    if (!st.anyAltDown() && !st.anyCtrlDown() && !st.anyShiftDown()) {
        m_camera_controller->update(input);
    }
}

void SceneTab::drawUIImpl() {
    updateSceneEditContext();

    ViewRecord* view = m_view_manager.resolve(m_view_id);
    DEV_ASSERT(view);

    auto change_tool = [this]() {
        auto selection = m_editor_services.selection().primary(m_doc_id);
        if (selection.scene == m_preview_scene_id) {
            if (Scene* scene = getResolvedScene()) {
                if (scene->has(TileMapLayerComponent_Id, selection.entity)) {
                    m_current_tool = SceneViewToolType::TilePaint;
                    m_tile_paint_tool->setLayerId(selection.entity);
                    return;
                }
            }
        }
        m_current_tool = SceneViewToolType::Select;
    };
    change_tool();

    updateRect(view->display_rect_os);
    drawMainView(view->display_rect_os);

    if (!m_editor.isPlaying()) {
        activeTool()->draw(view->display_rect_os);
    }

    submitView();
}

bool SceneTab::onAssetDropped(AssetHandle handle) {
    if (ViewTabBase::onAssetDropped(handle)) {
        return true;
    }

    IAsset* asset = handle.get();
    switch (asset->type()) {
        case AssetType::Prefab: {
            Scene* scene = getResolvedScene();
            m_editor_services.edit().submit(m_doc_id, [&](SceneCommandWriter& writer) {
                Entity ent = writer.createEntity();
                writer.addComponent(ent, PrefabInstanceComponent_Id);
                writer.addComponent(ent, HierarchyComponent_Id);
                writer.setProperty(ent, PrefabInstanceComponent_Id, CAVE_SID("prefab_id"), handle.guid());
                writer.attachChild(ent, scene->hierarchy().firstRoot().unwrap_or(Entity::null()));
                return ent;
            });

            return true;
        }
        default:
            return false;
    }
}

Scene* SceneTab::getResolvedScene() {
    return m_engine_services.sceneRegistry().resolve(m_preview_scene_id);
}

ISceneViewTool* SceneTab::activeTool() {
    return m_scene_tools[std::to_underlying(m_current_tool)].get();
}

}  // namespace cave
