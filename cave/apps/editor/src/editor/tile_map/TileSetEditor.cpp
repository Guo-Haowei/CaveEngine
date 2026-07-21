#include "TileSetEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/display/ICanvas.h"

#include "editor/services/DocumentService.h"

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

    m_camera_controller->setMoveSpeed(0.2f * CameraController2DEditor::kDefaultPanSpeed);
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

void TileSetEditor::drawTiles() {
    ICanvas& canvas = m_engine_services.canvas();

    auto assets = getAssets();
    if (assets.image && assets.tile_set) {
        canvas.pushView(m_view_id);

        const auto w = 0.5f * assets.tile_set->col();
        const auto h = 0.5f * assets.tile_set->row();

        Box2 box = Box2::fromCenterHalfExtent(Vec2f::Zero, Vec2f(w, h));

        canvas.addImage(assets.image->gpu_texture.get(),
                        box.min(),
                        box.max());
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
    const int column = tile_set.col();
    const int row = tile_set.row();
    m_sprite_selector.SelectSprite(*assets.image, &column, &row);
}

}  // namespace cave
