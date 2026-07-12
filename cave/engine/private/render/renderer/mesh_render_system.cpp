#include "RenderScene.h"

#include "cave/runtime/ecs/components/MaterialComponent.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/ecs/components/LightComponent.h"
#include "cave/runtime/ecs/components/SkeletalAnimationComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

// @TODO: enventually get rid of this file
namespace cave::render {

using namespace cave::math;

// @TODO: fix this function OMG
static void FillMaterialConstantBuffer(bool p_is_opengl,
                                       const MaterialComponent* p_material,
                                       MaterialConstantBuffer& cb) {
    cb.c_hasBaseColorMap = false;
    cb.c_hasNormalMap = false;
    cb.c_hasMaterialMap = false;
    cb.c_hasHeightMap = false;

    cb.c_baseColorMapHandle = 0;
    cb.c_normalMapHandle = 0;
    cb.c_materialMapHandle = 0;
    cb.c_heightMapHandle = 0;

    if (!p_material) {
        cb.c_baseColor = Vec4f(1, 0, 1, 1);
        cb.c_metallic = 0.5f;
        cb.c_roughness = 0.5f;
        cb.c_emissivePower = 0.0f;
        return;
    }

    cb.c_baseColor = p_material->base_color;
    cb.c_metallic = p_material->metallic;
    cb.c_roughness = p_material->roughness;
    cb.c_emissivePower = p_material->emissive;

    // @TODO: [SCRUM-210] fix material
    const auto& images = p_material->m_images;
    unused(p_is_opengl);
    auto set_texture = [&](TextureSlot p_idx,
                           TextureHandle& p_out_handle) {
        const int idx = std::to_underlying(p_idx);

        p_out_handle = 0;

        if ((int)images.size() <= idx) {
            return false;
        }

        const ImageAsset* image = images[idx].get();
        if (!image) {
            return false;
        }

        auto texture = reinterpret_cast<GpuTexture*>(image->gpu_texture.get());
        if (!texture) {
            return false;
        }

        p_out_handle = texture->GetHandle();
        return true;
    };

    cb.c_hasBaseColorMap = set_texture(TextureSlot::Base, cb.c_baseColorMapHandle);
    cb.c_hasNormalMap = set_texture(TextureSlot::Normal, cb.c_normalMapHandle);
    cb.c_hasMaterialMap = set_texture(TextureSlot::MetallicRoughness, cb.c_materialMapHandle);
};

using FilterObjectFunc1 = std::function<bool(const RenderableHeader&)>;
using FilterObjectFunc2 = std::function<bool(const AABB&)>;

static void FillPass(const RenderScene& p_rs,
                     const Scene& p_es,
                     FilterObjectFunc1 p_filter,
                     const math::Frustum& p_frustum,
                     std::vector<DrawItem>& p_commands,
                     const ResolvedView& p_view,
                     FrameData& p_framedata,
                     bool p_no_mat) {

    for (const RenderableHeader& header : p_rs.m_renderables) {
        if (!p_filter(header)) continue;
        if (!p_frustum.intersects(header.world_bound)) continue;
        if (header.payload.kind != PayloadKind::Mesh) continue;
        if (header.payload.index == kInvalidPayload) continue;
        const MeshPayload& mesh = p_rs.m_meshes[header.payload.index];

        ecs::Entity skeleton_id = mesh.skeleton;
        PerBatchConstantBuffer batch_buffer;
        batch_buffer.c_worldMatrix = header.world;
        batch_buffer.c_meshFlag = skeleton_id.valid();

        DrawItem draw{};
        const auto& highlighted = p_view.highlight.entities;
        if (auto it = highlighted.find(header.owner); it != highlighted.end()) {
            draw.flags = STENCIL_FLAG_HIGHLIGHT;
        }

        if (skeleton_id.valid()) {
            const SkeletonComponent* skeleton = p_es.component<SkeletonComponent>(skeleton_id);
            if (skeleton) {
                DEV_ASSERT(skeleton->bone_transforms.size() <= MAX_BONE_COUNT);

                BoneConstantBuffer bone;
                memcpy(bone.c_bones, skeleton->bone_transforms.data(), sizeof(Mat4f) * skeleton->bone_transforms.size());

                // @TODO: better memory usage
                draw.bone_idx = p_framedata.boneCache.FindOrAdd(skeleton_id, bone);
            }
        }

        if (!mesh.gpu_mesh) {
            continue;
        }

        draw.mesh_data = mesh.gpu_mesh;
        draw.batch_idx = p_framedata.batchCache.FindOrAdd(header.owner, batch_buffer);

        // if pass doesn't need material, early return
        if (p_no_mat) {
            draw.mat_idx = -1;
            draw.index = { 0, mesh.index_count };
            p_commands.emplace_back(draw);
            continue;
        }

        // otherwise go through all the subsets
        for (size_t idx = 0; idx < mesh.subsets.size(); ++idx) {
            const auto& subset = mesh.subsets[idx];
            // @TODO: cache world bound
            AABB aabb2 = subset.local_bound;
            aabb2.applyMatrix(header.world);
            if (!p_frustum.intersects(aabb2)) continue;

            ecs::Entity material_id = mesh.materials[idx];
            const MaterialComponent* material = p_es.component<MaterialComponent>(material_id);

            MaterialConstantBuffer material_buffer;
            FillMaterialConstantBuffer(p_framedata.options.is_opengl,
                                       material,
                                       material_buffer);

            draw.index = { subset.index_offset, subset.index_count };
            draw.mat_idx = p_framedata.materialCache.FindOrAdd(material_id, material_buffer);

            p_commands.emplace_back(draw);
        }
    }
}

static void FillLightBuffer(const RenderScene& p_rs,
                            const Scene& p_scene,
                            const ResolvedView& p_view,
                            FrameData& p_framedata) {
    const uint32_t light_count = glm::min<uint32_t>((uint32_t)p_scene.count<LightComponent>(), MAX_LIGHT_COUNT);

    auto& cache = p_framedata.perFrameCache;
    cache.c_lightCount = light_count;

    [[maybe_unused]] auto& point_shadow_cache = p_framedata.pointShadowCache;

    int idx = 0;
    for (auto [light_entity, light_component] : p_scene.view<LightComponent>()) {
        const TransformComponent* light_transform = p_scene.component<TransformComponent>(light_entity);
        DEV_ASSERT(light_transform);

        // SHOULD BE THIS INDEX
        Light& light = cache.c_lights[idx];
        bool cast_shadow = light_component.CastShadow();
        light.cast_shadow = cast_shadow;
        light.type = static_cast<int>(light_component.GetType());
        const MaterialComponent& material = *p_scene.component<MaterialComponent>(light_entity);
        // @TODO: [SCRUM-210] fix material
        light.color = material.base_color.xyz;
        light.color *= material.emissive;
        switch (light.type) {
            case LIGHT_TYPE_INFINITE: {
                Mat4f light_local_matrix = light_transform->localMatrix();
                Vec3f light_dir((light_local_matrix * Vec4f(0, 0, 1, 1)).xyz);
                light_dir = normalize(light_dir);
                cache.c_sunPosition = light_dir;
                light.cast_shadow = cast_shadow;
                light.position = light_dir;

                // @TODO: add option to specify extent
                // @would be nice if can add debug draw
                AABB world_bound = light_component.GetShadowRegion();
                if (!world_bound.valid()) {
                    world_bound = p_scene.bound();
                }
                Vec3f center = world_bound.center();
                Vec3f extents = world_bound.size();

                const float size = 0.7f * max(extents.x, max(extents.y, extents.z));
                Vec3f tmp;
                tmp.set(&light_dir.x);
                light.view_matrix = LookAtRh(center + tmp * size, center, Vec3f::UnitY);

                if (p_framedata.options.is_opengl) {
                    light.projection_matrix = BuildOpenGlOrthoRH(-size, size, -size, size, -size, 3.0f * size);
                } else {
                    light.projection_matrix = BuildOrthoRH(-size, size, -size, size, -size, 3.0f * size);
                }

                PerPassConstantBuffer pass_constant;
                // @TODO: Build correct matrices
                pass_constant.c_projectionMatrix = light.projection_matrix;
                pass_constant.c_viewMatrix = light.view_matrix;
                p_framedata.shadowPasses[0].pass_idx = static_cast<int>(p_framedata.passCache.size());
                p_framedata.passCache.emplace_back(pass_constant);

                Frustum light_frustum(light.projection_matrix * light.view_matrix);
                FillPass(
                    p_rs,
                    p_scene,
                    [](const RenderableHeader& p_renderable) {
                        return p_renderable.HasFlag(RenderableFlags::CastShadow);
                    },
                    light_frustum,
                    p_framedata.commands[std::to_underlying(DrawPhase::Shadow)],
                    p_view,
                    p_framedata,
                    true);
            } break;
            case LIGHT_TYPE_POINT: {
                // @TODO: there's a bug in shadow map allocation
                light.atten_constant = light_component.GetAttenConstant();
                light.atten_linear = light_component.GetAttenLinear();
                light.atten_quadratic = light_component.GetAttenQuadratic();
                light.position = light_component.GetPosition();
                light.cast_shadow = cast_shadow;
                light.max_distance = light_component.GetMaxDistance();
                light.shadow_map_index = -1;
                // LOG_WARN("TODO: fix light");
#if 0
                [[maybe_unused]] const int shadow_map_index = light_component.GetShadowMapIndex();
                if (cast_shadow && shadow_map_index != -1) {
                    light.shadow_map_index = shadow_map_index;

                    Vec3f radiance(light.max_distance);
                    AABB aabb = AABB::FromCenterSize(light.position, radiance);
                    auto pass = MakeOwner<PassContext>();
                    FillPass(
                        p_scene,
                        *pass.get(),
                        [](const ObjectComponent& p_object) {
                            return p_object.flags & ObjectComponent::FLAG_CAST_SHADOW;
                        },
                        [&](const AABB& p_aabb) {
                            return p_aabb.Intersects(aabb);
                        },
                        p_out_data, false);

                    DEV_ASSERT_INDEX(shadow_map_index, MAX_POINT_LIGHT_SHADOW_COUNT);
                    const auto& light_matrices = light_component.GetMatrices();
                    for (int face_id = 0; face_id < 6; ++face_id) {
                        const uint32_t slot = shadow_map_index * 6 + face_id;
                        point_shadow_cache[slot].c_pointLightMatrix = light_matrices[face_id];
                        point_shadow_cache[slot].c_pointLightPosition = light_component.GetPosition();
                        point_shadow_cache[slot].c_pointLightFar = light_component.GetMaxDistance();
                    }

                    p_out_data.pointShadowPasses[shadow_map_index] = std::move(pass);
                } else {
                    light.shadow_map_index = -1;
                }
#endif
            } break;
            case LIGHT_TYPE_AREA: {
                Mat4f transform = light_transform->worldMatrix();
                constexpr float s = 0.5f;
                light.points[0] = transform * Vec4f(-s, +s, 0.0f, 1.0f);
                light.points[1] = transform * Vec4f(-s, -s, 0.0f, 1.0f);
                light.points[2] = transform * Vec4f(+s, -s, 0.0f, 1.0f);
                light.points[3] = transform * Vec4f(+s, +s, 0.0f, 1.0f);
            } break;
            default:
                CRASH_NOW();
                break;
        }
        ++idx;
    }
}

static void FillMainPass(const Scene& p_es,
                         const RenderScene& p_rs,
                         const ResolvedView& p_view,
                         FrameData& p_framedata) {
    const CameraParams& camera = p_view.cam;
    const Frustum& frustum = p_view.frustum;

    // main pass
    PerPassConstantBuffer pass_constant;
    pass_constant.c_viewMatrix = camera.view;
    pass_constant.c_projectionMatrix = camera.proj;

    p_framedata.mainPass.pass_idx = static_cast<int>(p_framedata.passCache.size());
    p_framedata.passCache.emplace_back(pass_constant);

    FillPass(
        p_rs,
        p_es,
        [](const RenderableHeader& p_header) {
            // only draw visible opaque objects for pre pass
            return p_header.HasFlag(RenderableFlags::Visible) && !p_header.HasFlag(RenderableFlags::Transparent);
        },
        frustum,
        p_framedata.commands[std::to_underlying(DrawPhase::DepthPrepass)],
        p_view,
        p_framedata,
        true);

    FillPass(
        p_rs,
        p_es,
        [](const RenderableHeader& p_header) {
            // only draw visible opaque objects for deferred pass
            return p_header.HasFlag(RenderableFlags::Visible) && !p_header.HasFlag(RenderableFlags::Transparent);
        },
        frustum,
        p_framedata.commands[std::to_underlying(DrawPhase::Deferred)],
        p_view,
        p_framedata,
        false);

    FillPass(
        p_rs,
        p_es,
        [](const RenderableHeader& p_header) {
            return p_header.HasFlag(RenderableFlags::Visible) && p_header.HasFlag(RenderableFlags::Transparent);
        },
        frustum,
        p_framedata.commands[std::to_underlying(DrawPhase::Forward)],
        p_view,
        p_framedata,
        false);
}

void runMeshRenderSystem(const Scene& p_scene,
                         const RenderScene& p_rscene,
                         const ResolvedView& p_view,
                         FrameData& p_framedata) {
    FillLightBuffer(p_rscene, p_scene, p_view, p_framedata);
    FillMainPass(p_scene, p_rscene, p_view, p_framedata);
}

// @TODO: fix emitter
#if 0
static void FillMeshEmitterBuffer(const Scene& p_scene,
                                  RenderSystem& p_out_data) {
    for (auto [id, emitter] : p_scene.m_MeshEmitterComponents) {
        auto transform = p_scene.GetComponent<TransformComponent>(id);
        auto mesh = p_scene.GetComponent<MeshComponent>(emitter.meshId);
        if (DEV_VERIFY(transform && mesh)) {
            PerBatchConstantBuffer batch_buffer;
            batch_buffer.c_worldMatrix = Mat4f(1);
            batch_buffer.c_meshFlag = MESH_HAS_INSTANCE;

            BatchContext draw;

            auto& position_buffer = p_out_data.boneCache.buffer;

            InstanceContext instance;
            instance.gpuMesh = mesh->gpuResource.get();
            instance.indexCount = (uint32_t)mesh->indices.size();
            instance.indexOffset = 0;
            instance.instanceCount = (uint32_t)emitter.aliveList.size();
            instance.batchIdx = p_out_data.batchCache.FindOrAdd(id, batch_buffer);
            instance.instanceBufferIndex = (int)position_buffer.size();
            auto material_id = mesh->subsets[0].material_id;
            auto material = p_scene.GetComponent<MaterialComponent>(material_id);
            DEV_ASSERT(material);
            MaterialConstantBuffer material_buffer;
            FillMaterialConstantBuffer(p_out_data.options.isOpengl, material, material_buffer);
            instance.materialIdx = p_out_data.materialCache.FindOrAdd(material_id, material_buffer);

            // @HACK: use bone cache
            DEV_ASSERT(instance.instanceCount <= MAX_BONE_COUNT);
            position_buffer.resize(position_buffer.size() + 1);
            auto& gpu_buffer = position_buffer.back();
            int i = 0;
            for (auto index : emitter.aliveList) {
                const auto& p = emitter.particles[index.v];

                Mat4f translation = Translate(p.position);
                Mat4f scale = Scale(Vec3f(p.scale));
                Mat4f rotation = glm::toMat4(glm::quat(glm::vec3(p.rotation.x, p.rotation.y, p.rotation.z)));
                gpu_buffer.c_bones[i++] = translation * rotation * scale;
            }

            p_out_data.instances.push_back(instance);
        }
    }
}

static void FillParticleEmitterBuffer(const Scene& p_scene,
                                      RenderSystem& p_out_data) {
    // @TODO: engine->get frame
    static int s_counter = -1;
    s_counter++;

    const auto view = p_scene.View<ParticleEmitterComponent>();
    for (auto [id, emitter] : view) {
        const uint32_t pre_sim_idx = emitter.GetPreIndex();
        const uint32_t post_sim_idx = emitter.GetPostIndex();
        EmitterConstantBuffer buffer;
        const TransformComponent* transform = p_scene.GetComponent<TransformComponent>(id);
        buffer.c_preSimIdx = pre_sim_idx;
        buffer.c_postSimIdx = post_sim_idx;
        buffer.c_elapsedTime = p_scene.m_timestep;
        buffer.c_lifeSpan = emitter.particleLifeSpan;
        buffer.c_seeds = Vec3f(Random::Float(), Random::Float(), Random::Float());
        buffer.c_emitterScale = emitter.particleScale;
        buffer.c_emitterPosition = transform->GetTranslation();
        buffer.c_particlesPerFrame = emitter.particlesPerFrame;
        buffer.c_emitterStartingVelocity = emitter.startingVelocity;
        buffer.c_emitterMaxParticleCount = emitter.maxParticleCount;
        buffer.c_emitterHasGravity = emitter.gravity;

        buffer.c_particleColor = emitter.color;
        buffer.c_emitterUseTexture = !emitter.texture.empty();
        buffer.c_subUvCounter = s_counter;

        p_out_data.emitterCache.push_back(buffer);
        p_out_data.emitters.emplace_back(emitter);
    }
}
#endif

}  // namespace cave::render
