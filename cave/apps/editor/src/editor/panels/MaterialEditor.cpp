#include "MaterialEditor.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/EntityFactory.h"

#include "editor/EditorState.h"
#include "editor/widgets/DragDrop.h"
#include "engine/private/ui/layout.h"

namespace cave {

#if 0
void MaterialEditor::OnDestroy() {
}

void MaterialEditor::OnActivateInternal() {
    // auto scene_manager = static_cast<EditorSceneManager*>(ISceneManager::GetSingletonPtr());
    // scene_manager->OpenTempScene(m_tmp_scene);
    DEV_ASSERT(0);
}

void MaterialEditor::DrawMainView(const CameraComponent& p_camera) {
    ViewerTab::DrawMainView(p_camera);
}

void MaterialEditor::DrawAssetInspector() {
    MaterialAsset* material = m_document->GetHandle<MaterialAsset>().Get();

    std::vector<AssetChildPanel> descs = {
        {
            "LayerOverview",
            480,
            [&]() {
                if (material) {
                    DrawTextureSlots(*material);
                }
            },
        },
        {
            "PaintTab",
            0,
            [&]() {
                m_editor.GetAssetInspector().DrawContentBrowser();
            },
        }
    };

    const float full_width = ImGui::GetContentRegionAvail().x;

    ui::DrawContents(full_width, descs);
}

void MaterialEditor::DrawTextureSlots(MaterialAsset& p_material) {
    const ImVec2 region_size(128, 128);

    for (size_t i = 0; i < p_material.textures.size(); ++i) {
        Guid& material = p_material.textures[i];

        auto handle = AssetRegistry::GetSingleton().FindByGuid<ImageAsset>(material).unwrap_or(Handle<ImageAsset>());

        const ImageAsset* image = handle.Get();

        auto checkerboard = m_editor.context.checkerboard;
        DEV_ASSERT(checkerboard && checkerboard->gpu_texture);

        ImGui::Text("%s texture: ", EnumTraits<TextureSlot>::ToString(static_cast<TextureSlot>(i)).data());
        if (image && image->gpu_texture) {
            ImGui::Image(image->gpu_texture->GetHandle(), ImVec2(128, 128));
        }

        if (auto _handle = DragDropTarget(AssetType::Image); _handle.is_some()) {
            material = _handle.unwrap_unchecked().GetGuid();
        }
    }
}

const std::vector<const ToolBarButtonDesc*> MaterialEditor::GetToolBarButtons() const {
    return {};
}
#endif

}  // namespace cave
