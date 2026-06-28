#include "EcsSystems.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/threading/JobSystem.h"

// @TODO: refactor
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/ecs/components/All.h"

namespace cave {

using namespace cave::math;

// @TODO: refactor
namespace {
template<typename T>
constexpr float Saturate(T x) { return math::min(T(1), math::max(T(0), x)); }
}  // namespace

[[maybe_unused]] static constexpr uint32_t SMALL_SUBTASK_GROUP_SIZE = 64;

#define JS_FORCE_PARALLEL_FOR(TYPE, CTX, INDEX, SUBCOUNT, BODY) \
    CTX.Dispatch(                                               \
        static_cast<uint32_t>(scene.count<TYPE>()),             \
        SUBCOUNT,                                               \
        [&](jobsystem::JobArgs args) { const uint32_t INDEX = args.jobIndex; do { BODY; } while(0); })

#define JS_NO_PARALLEL_FOR(TYPE, CTX, INDEX, SUBCOUNT, BODY)          \
    (void)(CTX);                                                      \
    for (size_t INDEX = 0; INDEX < scene.GetCount<TYPE>(); ++INDEX) { \
        BODY;                                                         \
    }

#if USING(ENABLE_JOB_SYSTEM)
#define JS_PARALLEL_FOR JS_FORCE_PARALLEL_FOR
#else
#define JS_PARALLEL_FOR JS_NO_PARALLEL_FOR
#endif

class SkeletalAnimationSystem {
public:
    static void Update(Scene& scene, size_t p_index, float p_timestep);
};

// @TODO: fix
#pragma warning(push)
#pragma warning(disable : 4996)

void SkeletalAnimationSystem::Update(Scene& scene, size_t p_index, float p_timestep) {
    SkeletalAnimationComponent& animation = scene.getComponentByIndex<SkeletalAnimationComponent>(p_index);

    if (!animation.IsPlaying()) {
        return;
    }

    for (const SkeletalAnimationChannel& channel : animation.m_channels) {
        if (channel.path == AnimationChannelPath::Count) {
            continue;
        }
        DEV_ASSERT(channel.sampler_index < (int)animation.m_samplers.size());
        const SkeletalAnimationSampler& sampler = animation.m_samplers[channel.sampler_index];

        int key_left = 0;
        int key_right = 0;
        float time_first = std::numeric_limits<float>::min();
        float time_last = std::numeric_limits<float>::min();
        float time_left = std::numeric_limits<float>::min();
        float time_right = std::numeric_limits<float>::max();

        for (int k = 0; k < (int)sampler.keyframe_times.size(); ++k) {
            const float time = sampler.keyframe_times[k];
            if (time < time_first) {
                time_first = time;
            }
            if (time > time_last) {
                time_last = time;
            }
            if (time <= animation.m_timer && time > time_left) {
                time_left = time;
                key_left = k;
            }
            if (time >= animation.m_timer && time < time_right) {
                time_right = time;
                key_right = k;
            }
        }

        if (animation.m_timer < time_first) {
            continue;
        }

        const float left = sampler.keyframe_times[key_left];
        const float right = sampler.keyframe_times[key_right];

        float t = 0;
        if (key_left != key_right) {
            t = (animation.m_timer - left) / (right - left);
        }
        t = Saturate(t);

        TransformComponent* targetTransform = scene.component<TransformComponent>(channel.target_id);
        DEV_ASSERT(targetTransform);
        auto dummy_mix = [](const Vec3f& a, const Vec3f& b, float t) {
            glm::vec3 tmp = glm::mix(glm::vec3(a.x, a.y, a.z), glm::vec3(b.x, b.y, b.z), t);
            return Vec3f(tmp.x, tmp.y, tmp.z);
        };
        auto dummy_mix_4 = [](const Vec4f& a, const Vec4f& b, float t) {
            glm::vec4 tmp = glm::mix(glm::vec4(a.x, a.y, a.z, a.w), glm::vec4(b.x, b.y, b.z, b.w), t);
            return Vec4f(tmp.x, tmp.y, tmp.z, tmp.w);
        };
        switch (channel.path) {
            case AnimationChannelPath::Scale: {
                DEV_ASSERT(sampler.keyframe_data.size() == sampler.keyframe_times.size() * 3);
                const Vec3f* data = (const Vec3f*)sampler.keyframe_data.data();
                const Vec3f& vLeft = data[key_left];
                const Vec3f& vRight = data[key_right];
                targetTransform->setScale(dummy_mix(vLeft, vRight, t));
                break;
            }
            case AnimationChannelPath::Translation: {
                DEV_ASSERT(sampler.keyframe_data.size() == sampler.keyframe_times.size() * 3);
                const Vec3f* data = (const Vec3f*)sampler.keyframe_data.data();
                const Vec3f& vLeft = data[key_left];
                const Vec3f& vRight = data[key_right];
                targetTransform->setTranslation(dummy_mix(vLeft, vRight, t));
                break;
            }
            case AnimationChannelPath::Rotation: {
                DEV_ASSERT(sampler.keyframe_data.size() == sampler.keyframe_times.size() * 4);
                const Vec4f* data = (const Vec4f*)sampler.keyframe_data.data();
                const Vec4f& vLeft = data[key_left];
                const Vec4f& vRight = data[key_right];
                targetTransform->setRotation(dummy_mix_4(vLeft, vRight, t));
                break;
            }
            default:
                CRASH_NOW();
                break;
        }
        targetTransform->setDirty();
    }

    if (animation.IsLooped() && animation.m_timer > animation.m_end) {
        animation.m_timer = animation.m_start;
    }

    if (animation.IsPlaying()) {
        animation.m_timer += p_timestep * animation.m_speed;
    }
}

static void UpdateHierarchy(Scene& p_scene, size_t p_index, float p_timestep) {
    unused(p_timestep);

    ecs::Entity self_id = p_scene.getEntityByIndex<HierarchyComponent>(p_index);
    TransformComponent* self_transform = p_scene.component<TransformComponent>(self_id);

    if (!self_transform) {
        return;
    }

    Mat4f world_matrix = self_transform->localMatrix();
    const HierarchyComponent* hierarchy = &p_scene.getComponentByIndex<HierarchyComponent>(p_index);
    ecs::Entity parent = hierarchy->parent_id;

    while (parent.IsValid()) {
        TransformComponent* parent_transform = p_scene.component<TransformComponent>(parent);
        if (DEV_VERIFY(parent_transform)) {
            world_matrix = parent_transform->localMatrix() * world_matrix;

            if ((hierarchy = p_scene.component<HierarchyComponent>(parent)) != nullptr) {
                parent = hierarchy->parent_id;
                // DEV_ASSERT(parent.IsValid() || );
            } else {
                parent.MakeInvalid();
            }
        } else {
            break;
        }
    }

    self_transform->setWorldMatrix(world_matrix);
    self_transform->setDirty(false);
}

static void UpdateSkeleton(Scene& p_scene, size_t p_index, float) {
    TransformComponent* transform = p_scene.component<TransformComponent>(p_scene.getEntityByIndex<SkeletonComponent>(p_index));
    DEV_ASSERT(transform);

    // The transform world matrices are in world space, but skinning needs them in skeleton-local space,
    //	so that the skin is reusable for instanced meshes.
    //	We remove the skeleton's world matrix from the bone world matrix to obtain the bone local transform
    //	These local bone matrices will only be used for skinning, the actual transform components for the bones
    //	remain unchanged.
    //
    //	This is useful for an other thing too:
    //	If a whole transform tree is transformed by some parent (even gltf import does that to convert from RH
    // to LH space) 	then the inverseBindMatrices are not reflected in that because they are not contained in
    // the hierarchy system. 	But this will correct them too.

    SkeletonComponent& skeleton = p_scene.getComponentByIndex<SkeletonComponent>(p_index);
    const Mat4f R = glm::inverse(transform->worldMatrix());
    const size_t numBones = skeleton.bone_collection.size();
    if (skeleton.bone_transforms.size() != numBones) {
        skeleton.bone_transforms.resize(numBones);
    }

    int idx = 0;
    for (ecs::Entity boneID : skeleton.bone_collection) {
        const TransformComponent* boneTransform = p_scene.component<TransformComponent>(boneID);
        DEV_ASSERT(boneTransform);

        const Mat4f& B = skeleton.inverse_bind_matrices[idx];
        const Mat4f& W = boneTransform->worldMatrix();
        const Mat4f M = R * W * B;
        skeleton.bone_transforms[idx] = M;
        ++idx;

        // @TODO: skeleton animation
    }
};

static void UpdateLight(float p_timestep,
                        const TransformComponent& p_transform,
                        LightComponent& p_light) {
    unused(p_timestep);

    p_light.SetPosition(p_transform.translation());

    if (p_light.IsDirty() || p_transform.dirty()) {
        const float constant = p_light.GetAttenConstant();
        const float linear = p_light.GetAttenLinear();
        const float quadratic = p_light.GetAttenQuadratic();
        // update max distance
        constexpr float atten_factor_inv = 1.0f / 0.03f;
        if (linear == 0.0f && quadratic == 0.0f) {
            p_light.SetMaxDistance(1000.0f);
        } else {
            // (constant + linear * x + quad * x^2) * atten_factor = 1
            // quad * x^2 + linear * x + constant - 1.0 / atten_factor = 0
            const float a = quadratic;
            const float b = linear;
            const float c = constant - atten_factor_inv;

            float discriminant = b * b - 4 * a * c;
            if (discriminant < 0.0f) {
                CRASH_NOW_MSG("TODO: fix");
            }

            float sqrt_d = glm::sqrt(discriminant);
            float root1 = (-b + sqrt_d) / (2 * a);
            float root2 = (-b - sqrt_d) / (2 * a);
            float max_distance = root1 > 0.0f ? root1 : root2;
            max_distance = glm::max(LIGHT_SHADOW_MIN_DISTANCE + 1.0f, max_distance);
            p_light.SetMaxDistance(max_distance);
        }

        // update shadow map
        if (p_light.CastShadow()) {
            // @TODO: [SCRUM-178] shadow atlas
        }

        // update light space matrices
        if (p_light.CastShadow()) {
            switch (p_light.GetType()) {
                case LightType::Point: {
                    CRASH_NOW();
#if 0
                    constexpr float near_plane = LIGHT_SHADOW_MIN_DISTANCE;
                    const float far_plane = p_light.m_maxDistance;
                    const bool is_opengl = IRenderDevice::singleton().GetBackend() == Backend::OPENGL;
                    auto matrices = is_opengl ? BuildOpenGlPointLightCubeMapViewProjectionMatrix(p_light.m_position, near_plane, far_plane)
                                              : BuildPointLightCubeMapViewProjectionMatrix(p_light.m_position, near_plane, far_plane);

                    for (size_t i = 0; i < matrices.size(); ++i) {
                        p_light.m_lightSpaceMatrices[i] = matrices[i];
                    }
#endif
                } break;
                default:
                    break;
            }
        }

        // @TODO: don't update shadow map unless necessary
        p_light.SetDirty(false);
    }
}

void RunLightUpdateSystem(Scene& p_scene, jobsystem::Context&, float p_timestep) {
    CAVE_PROFILE_EVENT();

    auto view = p_scene.view<LightComponent, TransformComponent>();
    for (auto [id, light, transform] : view) {
        UpdateLight(p_timestep, transform, light);
    }
}

void RunTransformationUpdateSystem(Scene& scene, jobsystem::Context& p_context, float) {
    CAVE_PROFILE_EVENT();

    JS_PARALLEL_FOR(TransformComponent, p_context, index, SMALL_SUBTASK_GROUP_SIZE, {
        if (scene.getComponentByIndex<TransformComponent>(index).updateTransform()) {
            scene.m_dirtyFlags.fetch_or(SCENE_DIRTY_WORLD);
        }
    });
}

void RunAnimationUpdateSystem(Scene& scene, jobsystem::Context& p_context, float p_timestep) {
    CAVE_PROFILE_EVENT();
    JS_PARALLEL_FOR(SkeletalAnimationComponent, p_context, index, 1, SkeletalAnimationSystem::Update(scene, index, p_timestep));
}

void RunSkeletonUpdateSystem(Scene& scene, jobsystem::Context& p_context, float p_timestep) {
    CAVE_PROFILE_EVENT();
    JS_PARALLEL_FOR(SkeletonComponent, p_context, index, 1, UpdateSkeleton(scene, index, p_timestep));
}

void RunHierarchyUpdateSystem(Scene& scene, jobsystem::Context& p_context, float p_timestep) {
    CAVE_PROFILE_EVENT();
    JS_PARALLEL_FOR(HierarchyComponent, p_context, index, SMALL_SUBTASK_GROUP_SIZE, UpdateHierarchy(scene, index, p_timestep));
}

void RunMeshAABBUpdateSystem(Scene& scene, jobsystem::Context&, float) {
    CAVE_PROFILE_EVENT();

    AABB bound;

    for (auto [id, mesh_renderer] : scene.view<MeshRendererComponent>()) {
        if (!scene.has<TransformComponent>(id)) {
            continue;
        }

        const TransformComponent& transform = *scene.component<TransformComponent>(id);
        const MeshAsset* mesh = mesh_renderer.GetMeshHandle().get();
        if (!mesh) {
            continue;
        }

        Mat4f M = transform.worldMatrix();
        AABB aabb = mesh->localBound;
        aabb.ApplyMatrix(M);
        bound.UnionBox(aabb);
    }

    scene.m_bound = bound;
}

void RunFacingUpdateSystem(Scene& scene, jobsystem::Context&, float) {
    auto view = scene.view<FacingComponent, VelocityComponent, TransformComponent>();
    for (auto [ent, facing, vel, transform] : view) {
        const float x_speed = vel.linear.x;

        if (x_speed < 0.0f) {
            facing.facing = Facing::Left;
        } else if (x_speed > 0.0f) {
            facing.facing = Facing::Right;
        }
        switch (facing.facing) {
            case Facing::Left: {
                transform.setRotation(Vec4f{ 0, 1, 0, 0 });
            } break;
            case Facing::Right: {
                transform.setRotation(Vec4f{ 0, 0, 0, 1 });
            } break;
            default:
                break;
        }
    }
}

#if 0
void RunParticleEmitterUpdateSystem(Scene& p_scene, jobsystem::Context& p_context, float) {
    CAVE_PROFILE_EVENT();
    unused(p_context);

    for (auto [entity, emitter] : p_scene.m_ParticleEmitterComponents) {
        emitter.aliveBufferIndex = 1 - emitter.aliveBufferIndex;
    }
}

static void UpdateMeshEmitter(float p_timestep,
                              const TransformComponent& p_transform,
                              MeshEmitterComponent& p_emitter) {
    // initialize
    if (p_emitter.particles.empty()) {
        p_emitter.Reset();
    }

    if (!p_emitter.IsRunning()) {
        return;
    }

    // 1. emit new particles
    const int emission_count = min(p_emitter.emissionPerFrame, (int)p_emitter.deadList.size());
    p_emitter.aliveList.reserve(p_emitter.aliveList.size() + emission_count);
    const auto& position = p_transform.GetTranslation();
    for (int i = 0; i < emission_count; ++i) {
        DEV_ASSERT(!p_emitter.deadList.empty());
        const auto free_index = p_emitter.deadList.back();
        p_emitter.deadList.pop_back();
        p_emitter.aliveList.push_back(free_index);

        DEV_ASSERT(free_index.v < p_emitter.particles.size());
        auto& p = p_emitter.particles[free_index.v];

        Vector3f initial_speed{ 0 };
        initial_speed.x += Random::Float(p_emitter.vxRange.x, p_emitter.vxRange.y);
        initial_speed.y += Random::Float(p_emitter.vyRange.x, p_emitter.vyRange.y);
        initial_speed.z += Random::Float(p_emitter.vzRange.x, p_emitter.vzRange.y);
        Vector3f initial_rotation{
            Random::Float(-HalfPi(), HalfPi()),
            Random::Float(-HalfPi(), HalfPi()),
            Random::Float(-HalfPi(), HalfPi()),
        };

        p.Init(Random::Float(p_emitter.lifetimeRange.x, p_emitter.lifetimeRange.y),
               position,
               initial_speed,
               initial_rotation,
               p_emitter.scale);
    }

    // 2. update alive ones
    for (const auto index : p_emitter.aliveList) {
        p_emitter.UpdateParticle(index, p_timestep);
    }

    // 3. recycle
    std::vector<MeshEmitterComponent::Index> tmp;
    tmp.reserve(p_emitter.aliveList.size());
    const bool recycle = p_emitter.IsRecycle();
    for (int i = (int)p_emitter.aliveList.size() - 1; i >= 0; --i) {
        const auto index = p_emitter.aliveList[i];
        auto& p = p_emitter.particles[index.v];
        if (p.lifespan <= 0.0f) {
            p_emitter.aliveList.pop_back();
            if (recycle) {
                p_emitter.deadList.push_back(index);
            }
        } else {
            tmp.push_back(index);
        }
    }
    p_emitter.aliveList = std::move(tmp);
}

void RunMeshEmitterUpdateSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep) {
    CAVE_PROFILE_EVENT();

    unused(p_context);
    for (auto [id, emitter] : p_scene.m_MeshEmitterComponents) {
        const TransformComponent* transform = p_scene.GetComponent<TransformComponent>(id);
        if (DEV_VERIFY(transform)) {
            UpdateMeshEmitter(p_timestep, *transform, emitter);
        }
    }
}
#endif

}  // namespace cave
