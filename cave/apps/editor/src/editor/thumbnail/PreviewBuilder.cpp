#include "PreviewBuilder.h"

#include "cave/runtime/ecs/components/MaterialComponent.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneEdit.h"

#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/scene/EntityFactory.h"

namespace cave {

using math::Matrix4x4f;
using math::Vector3f;
using math::Vector4f;
using render::CameraSource;

static CameraComponent FitAABBToCamera(const math::AABB& p_aabb,
                                       const PreviewOptions& p_options,
                                       float p_padding = 1.15f) {
    CameraComponent camera;
    const Vector3f center = p_aabb.Center();
    const Vector3f extents = p_aabb.HalfExtent();

    const float r = p_padding * math::length(extents);
    const float aspect = (float)p_options.width / p_options.height;

    Matrix4x4f rotation = math::Rotate(math::Degree(-30.0f), Vector3f::UnitX);
    Vector3f front = (rotation * Vector4f::UnitZ).xyz;

    const float theta_y = 0.5f * glm::radians<float>(p_options.fov_y_deg);
    const float theta_x = std::atan(std::tan(theta_y) * aspect);

    const float dist_y = r / std::tan(theta_y);
    const float dist_x = r / std::tan(theta_x);
    float dist = glm::max(dist_x, dist_y);
    dist *= 1.02f;

    const Vector3f pos = center + front * dist;
    Matrix4x4f transform = math::Translate(pos);

    const float near_z = std::max(0.01f, dist - r * 2.0f);
    const float far_z = dist + r * 2.0f;

    camera.SetNear(near_z);
    camera.SetFar(far_z);
    camera.SetAspect((float)p_options.width / (float)p_options.height);
    camera.SetFovy(p_options.fov_y_deg);

    camera.Update(transform * rotation);
    return camera;
}

PreviewBuilder::PreviewBuilder(IApplication& p_app) noexcept
    : m_asset_reg(*p_app.GetAssetRegistry())
    , m_scene_reg(*p_app.GetSceneRegistry()) {}

PreviewBuilder::~PreviewBuilder() = default;

PreviewBuildResult PreviewBuilder::Build(const PreviewBuildRequest& p_req) const {
    AssetHandle handle = m_asset_reg.FindByGuid(p_req.guid).unwrap();
    switch (handle.GetMeta()->type) {
        case AssetType::Scene:
            return BuildScene(handle, p_req.options);
        case AssetType::Mesh:
            return BuildMesh(handle, p_req.options);
        case AssetType::Material:
            return BuildMaterial(handle, p_req.options);
        default:
            return { PreviewBuildStatus::Error };
    }
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
    SceneId scene_id = m_scene_reg.Register({ "thumbnail scene" }, std::move(scene));

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = scene_id,
        .camera = CameraSource::FirstCamera(),
    };
}

PreviewBuildResult PreviewBuilder::BuildMaterial(const AssetHandle& p_handle,
                                                 const PreviewOptions& p_options) const {

    auto scene = std::make_unique<Scene>();
    SceneEdit edit(*scene);

    auto root = EntityFactory::CreateTransformEntity(*scene, "root");
    scene->m_root = root;

    // add sphere
    {
        auto id = EntityFactory::CreateSphereEntity(*scene, "sphere");
        MeshRendererComponent* renderer = scene->GetComponent<MeshRendererComponent>(id);
        DEV_ASSERT(renderer);
        auto material_id = renderer->GetMaterialInstances()[0];
        MaterialComponent* material = scene->GetComponent<MaterialComponent>(material_id);
        material->SetResourceGuid(p_handle.GetGuid());
        edit.AttachChild(id);
    }

    // add point light (TODO: maybe area light?)
    {
        auto id = EntityFactory::CreatePointLightEntity(*scene, "light", math::Vector3f(0, 3, 1));
        edit.AttachChild(id);
    }

    scene->Update(0.0f);

    Matrix4x4f transform = math::Translate(Vector3f(0, 0, 1.5f));

    CameraComponent camera{};
    camera.SetAspect((float)p_options.width / (float)p_options.height);
    camera.Update(transform);
    camera.SetFovy(p_options.fov_y_deg);

    // @TODO: delete scene
    SceneId scene_id = m_scene_reg.Register({ "thumbnail mat" }, std::move(scene));

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = scene_id,
        .camera = CameraSource::External(camera),
    };
}

PreviewBuildResult PreviewBuilder::BuildMesh(const AssetHandle& p_handle, const PreviewOptions& p_options) const {
    auto scene = std::make_unique<Scene>();
    SceneEdit edit(*scene);
    auto root = EntityFactory::CreateTransformEntity(*scene, "root");
    scene->m_root = root;

    const MeshAsset* mesh = p_handle.Get<MeshAsset>();
    DEV_ASSERT(mesh);

    // add mesh
    {
        auto id = EntityFactory::CreateTransformEntity(*scene, "mesh");
        MeshRendererComponent& renderer = scene->Create<MeshRendererComponent>(id);
        renderer.SetResourceGuid(p_handle.GetGuid());
        renderer.OnDeserialized();
        edit.AttachChild(id);
    }

    // add point light (TODO: maybe area light?)
    {
        auto id = EntityFactory::CreatePointLightEntity(*scene, "light", math::Vector3f(0, 3, 1));
        edit.AttachChild(id);
    }

    scene->Update(0.0f);

    CameraComponent camera = FitAABBToCamera(mesh->localBound, p_options);

    // @TODO: delete scene
    SceneId scene_id = m_scene_reg.Register({ "thumbnail mesh" }, std::move(scene));

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = scene_id,
        .camera = CameraSource::External(camera),
    };
}

}  // namespace cave
