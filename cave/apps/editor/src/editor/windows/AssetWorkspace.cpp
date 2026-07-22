#include "AssetWorkspace.h"

#include "editor/services/EditorServices.h"
#include "editor/services/DocumentService.h"
#include "editor/services/IconCache.h"
#include "editor/services/Workspace.h"

namespace cave {

using namespace ::cave::math;

namespace {

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

}  // namespace

AssetWorkspace::AssetWorkspace(EditorState& editor)
    : EditorWindow(editor) {
}

void AssetWorkspace::onAttach() {
}

void AssetWorkspace::drawUIImpl() {
    if (!ImGui::BeginTabBar("##AssetTools")) {
        return;
    }

    if (ImGui::BeginTabItem("TileSet")) {
        // drawTileSetWorkspace();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("TileMap")) {
        // drawTileMapWorkspace();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Animation")) {
        // drawAnimationWorkspace();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Physics")) {
        // drawPhysicsWorkspace();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}

}  // namespace cave
