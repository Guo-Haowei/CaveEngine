#include "asset_inspector.h"

#include "editor/editor_layer.h"
#include "editor/viewer/viewer.h"
#include "editor/viewer/viewer_tab.h"

namespace cave {

AssetInspector::AssetInspector(EditorLayer& p_editor)
    : EditorWindow(p_editor) {
}

void AssetInspector::OnAttach() {
}

#if 0
void AssetInspector::InspectSprite(IAsset* p_asset) {
    auto sprite = dynamic_cast<SpriteAsset*>(p_asset);
    if (!sprite) {
        return;
    }

    std::vector<AssetChildPanel> descs = {
        {
            "ImageSource",
            360.0f,
            [&]() {
                ImGui::Text("Image");
                const float w = 300;
                auto& handle = sprite->GetHandle();

                // @TODO: checker board
                if (handle.IsReady()) {
                    const ImageAsset* asset = handle.Get();
                    DEV_ASSERT(asset);
                    const float h = w / asset->width * asset->height;
                    ImVec2 size = ImVec2(w, h);

                    ImGui::Image(asset->gpu_texture->GetHandle(), size);
                } else {
                    ImVec2 size = ImVec2(w, w);
                    ImGui::InvisibleButton("DropTarget", size);
                }

                DragDropTarget(AssetType::Image, [&](AssetHandle& p_handle) {
                    DEV_ASSERT(p_handle.GetMeta()->type == AssetType::Image);
                    sprite->SetImage(p_handle.GetGuid());
                });
            },
        },
        {
            "SpriteEditor",
            360.0f,
            [&]() { EditSprite(*sprite); },
        },
        {
            "TileSetPanel",
            0.0f,
            [&]() { TilePaint(*sprite); },
        },
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    DrawContents(full_width, descs);
}
#endif

void AssetInspector::UpdateInternal(Scene*) {
    if (ViewerTab* tab = m_editor.GetViewer().GetActiveTab(); tab) {
        tab->DrawAssetInspector();
    }
}

}  // namespace cave
