#include "MaterialDocument.h"

#include "cave/runtime/scene/SceneEdit.h"

#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/EntityFactory.h"

namespace cave {

MaterialDocument::MaterialDocument(IApplication& p_app, const Guid& p_guid)
    : DocumentBase(p_app, p_guid) {
    auto scene = std::make_unique<Scene>();
    SceneEdit edit(*scene);

    auto root = EntityFactory::CreateTransformEntity(*scene, "material_test");
    scene->m_root = root;

    if constexpr (1) {  // add point light
        auto id = EntityFactory::CreatePointLightEntity(*scene, "point_light", math::Vector3f(0, 3, 1));

        edit.AttachChild(id);
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
        transform.SetTranslation(math::Vector3f(0.0f, 0.0f, 0.0f));

        edit.AttachChild(id);
    }

    if constexpr (0)  // add plane
    {
        auto id = EntityFactory::CreatePlaneEntity(*scene, "plane");
        TransformComponent& transform = *scene->GetComponent<TransformComponent>(id);
        transform.SetScale(math::Vector3f(5));
        transform.RotateX(math::Degree(-90.0f));

        edit.AttachChild(id);
    }

    m_preview_scene = m_scene_reg.Register({ "material doc" }, std::move(scene));
}

}  // namespace cave
