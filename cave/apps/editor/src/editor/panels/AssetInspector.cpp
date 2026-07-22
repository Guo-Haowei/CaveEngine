#include "AssetInspector.h"

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

    ImageCanvasDesc desc;
    desc.id = "##ImageCanvasTest";
    desc.texture = checkerboard_handle;

    // Logical size being tested.
    desc.image_size_px = {
        192.0f,
        64.0f,
    };

    // Fill the inspector width and use a fixed test height.
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
        const Vec2f point =
            result.mouse_image_px.unwrap_unchecked();

        ImGui::Text(
            "Pixel: %.1f, %.1f",
            point.x,
            point.y);
    } else {
        ImGui::TextUnformatted("Pixel: --, --");
    }

    ImGui::SameLine();

    ImGui::Text(
        "Zoom: %.0f%%",
        canvas.zoom() * 100.0f);
}

}  // namespace

AssetInspector::AssetInspector(EditorState& editor)
    : EditorWindow(editor) {
}

void AssetInspector::onAttach() {
}

void AssetInspector::drawUIImpl() {
    const IconCache& icons = m_editor_services.iconCache();

    const auto checkerboard = icons.getIconHandle(IconName::Checkerboard);

    if (!checkerboard) {
        ImGui::TextDisabled("Checkerboard image is unavailable");
        return;
    }

    DrawImageCanvasTest(m_image_canvas, (ImTextureID)checkerboard);

    ImGui::Separator();

#if 0
    Workspace& workspace = m_editor_services.workspace();
    DocId doc_id = workspace.focusedDoc();
    IDocument* doc = m_editor_services.document().resolve(doc_id);
    if (doc == nullptr) {
        return;
    }

    if (Tab* tab = workspace.focusedTab()) {
        tab->drawAssetInspector(*doc);
    }
#endif
}

}  // namespace cave
