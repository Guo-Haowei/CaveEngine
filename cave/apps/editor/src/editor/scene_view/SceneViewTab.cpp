#include "SceneViewTab.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

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

SceneViewTab::SceneViewTab(EditorState& editor,
                           DocId doc_id,
                           SceneId scene_id,
                           ViewDimension dim)
    : ViewTabBase(editor, doc_id, scene_id, dim)
    , m_editor(editor)
    , m_debug_id(MakeDebugId(this)) {

    m_play_button = {
        "SceneViewTab.play",
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
        "SceneViewTab.pause",
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
void SceneViewTab::submitView() {
    ViewTabBase::submitView(true);
}

void SceneViewTab::onCreate() {
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

    m_editor_services.picking().addConsumer(this);
}

void SceneViewTab::onDestroy() {
    ViewTabBase::onDestroy();

    m_editor_services.picking().removeConsumer(this);
}

Option<PickData> SceneViewTab::getPickData(const Vec2f& point_os) {
    if (!isVisible()) return None();

    return activeTool()->getPickData(point_os);
}

void SceneViewTab::drawToolbar() {
    std::array<const ToolbarButtonDesc*, 2> descs = {
        &m_play_button,
        &m_pause_button,
    };

    DrawToolbar(descs);
}

void SceneViewTab::onInputEvents(const InputFrame& input) {
    if (!isHovered()) {
        return;
    }

    if (m_editor.isPlaying()) {
        return;
    }

    activeTool()->onInputEvents(input);

    const KeyState& st = m_engine_services.inputService().keyState();
    if (!st.anyAltDown() && !st.anyCtrlDown() && !st.anyShiftDown()) {
        m_camera_controller->update(input);
    }
}

void SceneViewTab::drawUIImpl() {
    ViewRecord* view = m_view_manager.resolve(m_view_id);
    DEV_ASSERT(view);

    auto change_tool = [this]() {
        auto selection = m_editor_services.selection().primary(m_doc_id);
        if (selection.scene == m_preview_scene_id) {
            if (Scene* scene = getResolvedScene()) {
                if (scene->has(TileMapLayerComponent_Id, selection.entity)) {
                    m_current_tool = SceneViewToolType::TilePaint;
                    // @TODO: set it
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

bool SceneViewTab::onAssetDropped(AssetHandle handle) {
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

Scene* SceneViewTab::getResolvedScene() {
    return m_engine_services.sceneRegistry().resolve(m_preview_scene_id);
}

ISceneViewTool* SceneViewTab::activeTool() {
    return m_scene_tools[std::to_underlying(m_current_tool)].get();
}

}  // namespace cave
