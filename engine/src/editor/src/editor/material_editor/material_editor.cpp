#include "material_editor.h"

#include "engine/assets/image_asset.h"
#include "engine/assets/material_asset.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/material_editor/material_document.h"
#include "editor/panels/asset_inspector.h"
#include "editor/widgets/widget.h"

namespace cave {

MaterialEditor::MaterialEditor(EditorLayer& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer) {
    ViewerTab::CreateDefaultCamera3D(m_camera);

    m_camera.SetPosition(Vector3f(0, 0, 2));
}

void MaterialEditor::OnCreate(const Guid& p_guid) {
    ViewerTab::OnCreate(p_guid);

    m_document = std::make_shared<MaterialDocument>(p_guid);

    auto scene_manager = static_cast<EditorSceneManager*>(ISceneManager::GetSingletonPtr());
    DEV_ASSERT(scene_manager);

    m_tmp_scene = scene_manager->CreateTempScene(p_guid, [&]() {
        auto scene = std::make_shared<Scene>();
        auto root = EntityFactory::CreateTransformEntity(*scene, "material_test");
        scene->m_root = root;

        if constexpr (1) {  // add point light
            auto id = EntityFactory::CreatePointLightEntity(*scene, "point_light", Vector3f(0, 3, 1));

            scene->AttachChild(id);
        }

        if constexpr (1)  // add sphere
        {
            auto id = EntityFactory::CreateSphereEntity(*scene, "sphere");
            MeshRendererComponent* renderer = scene->GetComponent<MeshRendererComponent>(id);
            DEV_ASSERT(renderer);
            auto material_id = renderer->GetMaterialInstances()[0];
            MaterialComponent* material = scene->GetComponent<MaterialComponent>(material_id);
            material->SetResourceGuid(p_guid);

            TransformComponent& transform = *scene->GetComponent<TransformComponent>(id);
            transform.SetTranslation(Vector3f(0.0f, 0.0f, 0.0f));

            scene->AttachChild(id);
        }

        if constexpr (0)  // add plane
        {
            auto id = EntityFactory::CreatePlaneEntity(*scene, "plane");
            TransformComponent& transform = *scene->GetComponent<TransformComponent>(id);
            transform.SetScale(Vector3f(5));
            transform.RotateX(Degree(-90.0f));

            scene->AttachChild(id);
        }

        return scene;
    });
}

void MaterialEditor::OnDestroy() {
}

void MaterialEditor::OnActivate() {
    auto scene_manager = static_cast<EditorSceneManager*>(ISceneManager::GetSingletonPtr());
    scene_manager->OpenTempScene(m_tmp_scene);
}

Scene* MaterialEditor::GetScene() {
    return m_tmp_scene.get();
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

    DrawContents(full_width, descs);
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
        CenteredImage(image, region_size, checkerboard->gpu_texture->GetHandle());

        // if (ImGui::IsItemClicked()) {
        // }

        if (auto _handle = DragDropTarget(AssetType::Image); _handle.is_some()) {
            material = _handle.unwrap_unchecked().GetGuid();
        }
    }
}

bool MaterialEditor::HandleInput(const InputEvent* p_input_event) {
    unused(p_input_event);
    return false;
}

Document& MaterialEditor::GetDocument() const {
    return *m_document;
}

const CameraComponent& MaterialEditor::GetActiveCameraInternal() const {
    return m_camera;
}

const std::vector<const ToolBarButtonDesc*> MaterialEditor::GetToolBarButtons() const {
    return {};
}

}  // namespace cave
