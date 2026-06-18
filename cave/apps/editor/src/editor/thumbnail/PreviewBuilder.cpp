#include "PreviewBuilder.h"

#include "cave/runtime/ecs/components/MaterialComponent.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

using namespace ::cave::literals;
using namespace ::cave::math;
using ecs::Entity;

static CameraComponent FitAABBToCamera(const math::AABB& aabb,
                                       const PreviewOptions& options,
                                       float padding = 1.15f) {
    CameraComponent camera;
    const Vec3f center = aabb.Center();
    const Vec3f extents = aabb.HalfExtent();

    const float r = padding * math::length(extents);
    const float aspect = (float)options.width / options.height;

    Mat4f rotation = math::Rotate(math::Degree(-30.0f), Vec3f::UnitX);
    Vec3f front = (rotation * Vec4f::UnitZ).xyz;

    const float theta_y = 0.5f * glm::radians<float>(options.fov_y_deg);
    const float theta_x = std::atan(std::tan(theta_y) * aspect);

    const float dist_y = r / std::tan(theta_y);
    const float dist_x = r / std::tan(theta_x);
    float dist = glm::max(dist_x, dist_y);
    dist *= 1.02f;

    const Vec3f pos = center + front * dist;
    Mat4f transform = math::Translate(pos);

    const float near_z = std::max(0.01f, dist - r * 2.0f);
    const float far_z = dist + r * 2.0f;

    camera.SetNear(near_z);
    camera.SetFar(far_z);
    camera.SetAspect((float)options.width / (float)options.height);
    camera.SetFovy(options.fov_y_deg);

    camera.Update(transform * rotation);
    return camera;
}

PreviewBuilder::PreviewBuilder(EngineServices& services) noexcept
    : asset_reg_(services.assetRegistry())
    , scene_reg_(services.sceneRegistry()) {}

PreviewBuilder::~PreviewBuilder() = default;

PreviewBuildResult PreviewBuilder::build(const PreviewBuildRequest& req) const {
    AssetHandle handle = asset_reg_.FindByGuid(req.guid).unwrap();
    switch (handle.GetMeta()->type) {
        case AssetType::Scene:
            return buildScene(handle, req.options);
        case AssetType::Mesh:
            return buildMesh(handle, req.options);
        case AssetType::Material:
            return buildMaterial(handle, req.options);
        default:
            return { PreviewBuildStatus::Error };
    }
}

PreviewBuildResult PreviewBuilder::buildScene(const AssetHandle& handle,
                                              const PreviewOptions& options) const {
    unused(options);

    const Scene* source_scene = handle.Get<Scene>();
    DEV_ASSERT(source_scene);
    const AssetMetaData* meta = handle.GetMeta();
    DEV_ASSERT(meta);

    auto scene = std::make_unique<Scene>(std::format("{}-thumbnail", meta->name));
    scene->copy(*source_scene);

    for (auto [id, cam] : scene->view<CameraComponent>()) {
        cam.SetAspect(1.0f);
    }
    scene->update(0.0f);

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = scene_reg_.registerScene(std::move(scene)),
        .camera = CameraSource::FirstCamera(),
    };
}

PreviewBuildResult PreviewBuilder::buildMaterial(const AssetHandle& handle,
                                                 const PreviewOptions& options) const {

    SceneCommandWriter cb(asset_reg_);
    Entity root = cb.CreateRootObject();

    if constexpr (1) {
        Entity light = cb.CreatePointLightObject("light", math::Vec3f(0, 3, 1));
        cb.AttachChild(light, root);
    }

    if constexpr (1) {
        Guid guid = handle.GetGuid();
        Entity sphere = cb.CreateSphereObject("sphere", { &guid });
        cb.AttachChild(sphere, root);
    }

    const AssetMetaData* meta = handle.GetMeta();
    DEV_ASSERT(meta);
    auto scene = std::make_unique<Scene>(std::format("{}-thumbnail", meta->name));

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.GetAllocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });

    scene->m_root = map.Resolve(root);
    scene->update(0.0f);

    Mat4f transform = math::Translate(Vec3f(0, 0, 1.5f));

    CameraComponent camera{};
    camera.SetAspect((float)options.width / (float)options.height);
    camera.Update(transform);
    camera.SetFovy(options.fov_y_deg);

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = scene_reg_.registerScene(std::move(scene)),
        .camera = CameraSource::External(camera),
    };
}

PreviewBuildResult PreviewBuilder::buildMesh(const AssetHandle& handle, const PreviewOptions& options) const {

    SceneCommandWriter cb(asset_reg_);
    Entity root = cb.CreateRootObject();

    const MeshAsset* mesh = handle.Get<MeshAsset>();
    DEV_ASSERT(mesh);

    if constexpr (1) {
        Entity light = cb.CreatePointLightObject("light", math::Vec3f(0, 3, 1));
        cb.AttachChild(light, root);
    }

    if constexpr (1) {
        Entity e = cb.CreateTransformObject("mesh");
        cb.AddComponent(e, MeshRendererComponent_Id);
        cb.SetProperty(e, MeshRendererComponent_Id, "mesh_id"_sid, handle.GetGuid());
        cb.AttachChild(e, root);
    }

    const AssetMetaData* meta = handle.GetMeta();
    DEV_ASSERT(meta);
    auto scene = std::make_unique<Scene>(std::format("{}-thumbnail", meta->name));

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.GetAllocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });

    scene->m_root = map.Resolve(root);
    scene->update(0.0f);

    CameraComponent camera = FitAABBToCamera(mesh->localBound, options);

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = scene_reg_.registerScene(std::move(scene)),
        .camera = CameraSource::External(camera),
    };
}

}  // namespace cave
