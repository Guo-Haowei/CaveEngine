#include "PreviewBuilder.h"

#include "cave/runtime/ecs/components/MaterialComponent.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneRuntime.h"

#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/assets/PrefabAsset.h"
#include "engine/private/runtime/assets/SceneAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

using namespace ::cave::literals;
using namespace ::cave::math;
using ecs::Entity;

// @TODO: refactor
static CameraComponent FitAABBToCamera(const math::AABB& aabb,
                                       const PreviewOptions& options,
                                       float padding = 1.15f) {
    CameraComponent camera;
    const Vec3f center = aabb.center();
    const Vec3f extents = aabb.halfExtent();

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

    camera.setNear(near_z);
    camera.setFar(far_z);
    camera.setAspect((float)options.width / (float)options.height);
    camera.setFovy(options.fov_y_deg);

    camera.update(transform * rotation);
    return camera;
}

PreviewBuilder::PreviewBuilder(EngineServices& engine_services) noexcept
    : m_asset_reg(engine_services.assetRegistry())
    , m_scene_reg(engine_services.sceneRegistry())
    , m_engine_services(engine_services) {}

PreviewBuilder::~PreviewBuilder() = default;

PreviewBuildResult PreviewBuilder::build(const PreviewBuildRequest& req) const {
    AssetHandle handle = m_asset_reg.findByGuid(req.guid).unwrap();
    const AssetMetaData* meta = handle.meta();
    DEV_ASSERT(meta);
    switch (meta->type) {
        case AssetType::Scene: {
            if (const auto asset = handle.get<SceneAsset>()) {
                return buildSceneImpl(meta, asset->scene(), req.options);
            }
        } break;
        case AssetType::Prefab: {
            if (const auto asset = handle.get<PrefabAsset>()) {
                return buildSceneImpl(meta, asset->scene(), req.options);
            }
        } break;
        case AssetType::Mesh:
            return buildMesh(meta, handle, req.options);
        case AssetType::Material:
            return buildMaterial(meta, handle, req.options);
        default:
            break;
    }
    return { PreviewBuildStatus::Error };
}

PreviewBuildResult PreviewBuilder::buildSceneImpl(const AssetMetaData* meta,
                                                  const Scene& asset,
                                                  const PreviewOptions& options) const {
    auto scene = MakeOwner<Scene>();
    scene->copy(asset);

    // @TODO: better camera
    CameraSource camera_source;
    if (scene->count<CameraComponent>()) {
        for (auto [id, cam] : scene->view<CameraComponent>()) {
            cam.setAspect(1.0f);
        }

        camera_source = CameraSource::FirstCamera();
    } else {
        Mat4f transform = math::Translate(Vec3f(0, 0, 1.5f));
        CameraComponent camera{};
        camera.setAspect((float)options.width / (float)options.height);
        camera.update(transform);
        camera.setFovy(options.fov_y_deg);
        camera_source = CameraSource::External(camera);
    }

    scene->alwaysRun(MakeOwner<SceneRuntime>(
        SceneTickDomain::Editor,
        m_engine_services,
        *scene,
        ViewId{}));

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = m_scene_reg.registerScene(
            {
                .source = SceneSource::Thumbnail,
                .debug_name = meta->name,
            },
            std::move(scene)),
        .camera = camera_source,
    };
}

PreviewBuildResult PreviewBuilder::buildMaterial(const AssetMetaData* meta,
                                                 const AssetHandle& handle,
                                                 const PreviewOptions& options) const {

    SceneCommandWriter cb(m_asset_reg);
    Entity root = cb.rootObject();

    if constexpr (1) {
        Entity light = cb.pointLightObject("light", math::Vec3f(0, 3, 1));
        cb.attachChild(light, root);
    }

    if constexpr (1) {
        Guid guid = handle.guid();
        Entity sphere = cb.sphereObject("sphere", { &guid });
        cb.attachChild(sphere, root);
    }

    auto scene = MakeOwner<Scene>();

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.allocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });

    scene->setRoot(map.resolve(root));
    scene->update(0.0f);

    Mat4f transform = math::Translate(Vec3f(0, 0, 1.5f));

    CameraComponent camera{};
    camera.setAspect((float)options.width / (float)options.height);
    camera.update(transform);
    camera.setFovy(options.fov_y_deg);

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = m_scene_reg.registerScene(
            {
                .source = SceneSource::Thumbnail,
                .debug_name = meta->name,
            },
            std::move(scene)),
        .camera = CameraSource::External(camera),
    };
}

PreviewBuildResult PreviewBuilder::buildMesh(const AssetMetaData* meta,
                                             const AssetHandle& handle,
                                             const PreviewOptions& options) const {
    SceneCommandWriter cb(m_asset_reg);
    Entity root = cb.rootObject();

    const MeshAsset* mesh = handle.get<MeshAsset>();
    DEV_ASSERT(mesh);

    if constexpr (1) {
        Entity light = cb.pointLightObject("light", math::Vec3f(0, 3, 1));
        cb.attachChild(light, root);
    }

    if constexpr (1) {
        Entity e = cb.transformObject("mesh");
        cb.addComponent(e, MeshRendererComponent_Id);
        cb.setProperty(e, MeshRendererComponent_Id, "mesh_id"_sid, handle.guid());
        cb.attachChild(e, root);
    }

    auto scene = MakeOwner<Scene>();

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.allocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });

    scene->setRoot(map.resolve(root));
    scene->update(0.0f);

    CameraComponent camera = FitAABBToCamera(mesh->localBound, options);

    return {
        .status = PreviewBuildStatus::Ok,
        .scene_id = m_scene_reg.registerScene(
            {
                .source = SceneSource::Thumbnail,
                .debug_name = meta->name,
            },
            std::move(scene)),
        .camera = CameraSource::External(camera),
    };
}

}  // namespace cave
