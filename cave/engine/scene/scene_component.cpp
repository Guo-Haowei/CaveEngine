#include "scene_component.h"

#include "engine/core/io/archive.h"
#include "engine/math/matrix_transform.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/graphics_manager_interface.h"

namespace cave {

bool PrefabInstanceComponent::SetResourceGuid(const Guid& p_guid) {
    if (p_guid != m_prefab_id) {
        m_prefab_id = p_guid;
        return true;
    }
    return false;
}

#pragma region RIGID_BODY_COMPONENT
RigidBodyComponent& RigidBodyComponent::InitCube(const Vector3f& p_half_size) {
    shape = SHAPE_CUBE;
    size = p_half_size;
    return *this;
}

RigidBodyComponent& RigidBodyComponent::InitSphere(float p_radius) {
    shape = SHAPE_SPHERE;
    size = Vector3f(p_radius);
    return *this;
}

RigidBodyComponent& RigidBodyComponent::InitGhost() {
    objectType = GHOST;
    mass = 1.0f;
    return *this;
}
#pragma endregion RIGID_BODY_COMPONENT

#if 0
void MeshEmitterComponent::Reset() {
    if ((int)particles.size() != maxMeshCount) {
        particles.resize(maxMeshCount);
    }

    aliveList.clear();
    aliveList.reserve(maxMeshCount);
    deadList.clear();
    deadList.reserve(maxMeshCount);
    for (int i = 0; i < maxMeshCount; ++i) {
        deadList.emplace_back(i);
    }
}

void MeshEmitterComponent::UpdateParticle(Index p_index, float p_timestep) {
    DEV_ASSERT(p_index.v < particles.size());
    auto& p = particles[p_index.v];
    DEV_ASSERT(p.lifespan >= 0.0f);

    p.scale *= (1.0f - p_timestep);
    p.scale = max(p.scale, 0.1f);
    p.velocity += p_timestep * gravity;
    p.rotation += Vector3f(p_timestep);
    p.lifespan -= p_timestep;

    p.position += p_timestep * p.velocity;
}
#endif

// @TODO: refactor

void AnimationComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(flags);
    p_archive.ArchiveValue(start);
    p_archive.ArchiveValue(end);
    p_archive.ArchiveValue(timer);
    p_archive.ArchiveValue(amount);
    p_archive.ArchiveValue(speed);
    p_archive.ArchiveValue(channels);

    if (p_archive.IsWriteMode()) {
        uint64_t num_samplers = samplers.size();
        p_archive << num_samplers;
        for (uint64_t i = 0; i < num_samplers; ++i) {
            p_archive << samplers[i].keyframeTimes;
            p_archive << samplers[i].keyframeData;
        }
    } else {
        uint64_t num_samplers = 0;
        p_archive >> num_samplers;
        samplers.resize(num_samplers);
        for (uint64_t i = 0; i < num_samplers; ++i) {
            p_archive >> samplers[i].keyframeTimes;
            p_archive >> samplers[i].keyframeData;
        }
    }
}

void ArmatureComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(boneCollection);
    p_archive.ArchiveValue(inverseBindMatrices);
}

void CollisionObjectBase::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(collisionType);
    p_archive.ArchiveValue(collisionMask);
}

void RigidBodyComponent::Serialize(Archive& p_archive, uint32_t p_version) {
    CollisionObjectBase::Serialize(p_archive, p_version);

    p_archive.ArchiveValue(shape);
    p_archive.ArchiveValue(objectType);
    p_archive.ArchiveValue(size);
    p_archive.ArchiveValue(mass);
}

void EnvironmentComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(sky.type);
    p_archive.ArchiveValue(sky.texturePath);
    p_archive.ArchiveValue(ambient.color);
}

void VoxelGiComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(flags);
}

}  // namespace cave
