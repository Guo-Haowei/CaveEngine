#include "PreviewBuilder.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"
#include "engine/private/runtime/scene/EntityFactory.h"

namespace cave {

using math::Matrix4x4f;
using math::Vector3f;
using render::CameraSource;

PreviewBuilder::PreviewBuilder(IApplication& p_app) noexcept
    : m_asset_reg(*p_app.GetAssetRegistry())
    , m_scene_reg(*p_app.GetSceneRegistry()) {}

PreviewBuilder::~PreviewBuilder() = default;

PreviewBuildResult PreviewBuilder::Build(const PreviewBuildRequest& p_req) const {
    AssetHandle handle = m_asset_reg.FindByGuid(p_req.guid).unwrap();
    switch (handle.GetMeta()->type) {
        case AssetType::Scene: {
            return BuildScene(handle, p_req.options);
        } break;
        case AssetType::Material: {
            return BuildMaterial(handle, p_req.options);
        } break;
        default: {
        } break;
    }
    return { PreviewBuildStatus::Error };
}

PreviewBuildResult PreviewBuilder::BuildScene(const AssetHandle& p_handle,
                                              const PreviewOptions& p_options) const {
    unused(p_options);

    const Scene* source_scene = p_handle.Get<Scene>();
    DEV_ASSERT(source_scene);
    auto scene = std::make_unique<Scene>();
    scene->Copy(*source_scene);

    for (auto [id, cam] : scene->View<CameraComponent>()) {
        cam.SetAspect(1.0f);
    }
    scene->Update(0.0f);

    // @TODO: delete scene
    SceneId scene_id = m_scene_reg.Register(std::move(scene));

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = scene_id,
        .camera = CameraSource::FirstCamera(),
    };
}

PreviewBuildResult PreviewBuilder::BuildMaterial(const AssetHandle& p_handle,
                                                 const PreviewOptions& p_options) const {

    auto scene = std::make_unique<Scene>();
    auto root = EntityFactory::CreateTransformEntity(*scene, "material_test");
    scene->m_root = root;

    // add sphere
    {
        auto id = EntityFactory::CreateSphereEntity(*scene, "sphere");
        MeshRendererComponent* renderer = scene->GetComponent<MeshRendererComponent>(id);
        DEV_ASSERT(renderer);
        auto material_id = renderer->GetMaterialInstances()[0];
        MaterialComponent* material = scene->GetComponent<MaterialComponent>(material_id);
        material->SetResourceGuid(p_handle.GetGuid());

        TransformComponent& transform = *scene->GetComponent<TransformComponent>(id);
        transform.SetTranslation(Vector3f(0.0f, 0.0f, 0.0f));

        scene->AttachChild(id);
    }

    // add point light (TODO: maybe point light?)
    {
        auto id = EntityFactory::CreatePointLightEntity(*scene, "light", math::Vector3f(0, 3, 1));
        scene->AttachChild(id);
    }

    scene->Update(0.0f);

    Matrix4x4f transform = math::Translate(Vector3f(0, 0, 1.5f));

    CameraComponent camera{};
    camera.SetAspect((float)p_options.width / (float)p_options.height);
    camera.Update(transform);
    camera.SetFovy(p_options.fov_y_deg);

    // @TODO: delete scene
    SceneId scene_id = m_scene_reg.Register(std::move(scene));

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = scene_id,
        .camera = CameraSource::External(camera),
    };
}

}  // namespace cave
