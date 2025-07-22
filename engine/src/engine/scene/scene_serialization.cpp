#include <fstream>

#include "engine/assets/material_asset.h"
#include "engine/core/io/archive.h"
#include "engine/core/io/file_access.h"
#include "engine/core/string/string_utils.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/scene.h"
#include "engine/serialization/yaml_include.h"

namespace cave {

#define SCENE_DBG_LOG(...) LOG_VERBOSE(__VA_ARGS__)

#if 0
Result<void> SaveSceneBinary(const std::string& p_path, Scene& p_scene) {
    Archive archive;
    if (auto res = archive.OpenWrite(p_path); !res) {
        return CAVE_ERROR(res.error());
    }

    archive << SCENE_MAGIC;
    archive << LATEST_SCENE_VERSION;
    archive << ecs::Entity::GetSeed();
    archive << p_scene.m_root;
    archive << p_scene.m_physicsMode;

    archive << SCENE_GUARD_MESSAGE;

    for (const auto& it : p_scene.GetLibraryEntries()) {
        if (it.second.m_manager->GetCount()) {
            archive << HAS_NEXT_FLAG;
            archive << it.first;  // write name
            it.second.m_manager->Serialize(archive, LATEST_SCENE_VERSION);
        }
    }
    archive << uint64_t(0);
    return Result<void>();
}

Result<void> LoadSceneBinary(const std::string& p_path, Scene& p_scene) {
    Archive archive;
    if (auto res = archive.OpenRead(p_path); !res) {
        return CAVE_ERROR(res.error());
    }

    char magic[sizeof(SCENE_MAGIC)]{ 0 };
    if (!archive.Read(magic) || !StringUtils::StringEqual(magic, SCENE_MAGIC)) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "file corrupted, magic is not '{}'", SCENE_MAGIC);
    }

    uint32_t version;
    if (!archive.Read(version) || version > LATEST_SCENE_VERSION) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "incorrect scene version {}, current version is {}", version, LATEST_SCENE_VERSION);
    }

    SCENE_DBG_LOG("loading scene '{}', version: {}", p_path, version);

    uint32_t seed = ecs::Entity::MAX_ID;
    if (!archive.Read(seed)) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "failed to read seed");
    }

    archive >> p_scene.m_root;

    p_scene.m_physicsMode = PhysicsMode::NONE;
    if (version >= 19) {
        archive >> p_scene.m_physicsMode;
    }

    char guard_message[sizeof(SCENE_GUARD_MESSAGE)]{ 0 };
    archive >> guard_message;
    if (!StringUtils::StringEqual(guard_message, SCENE_GUARD_MESSAGE)) {
        return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT);
    }

    for (;;) {
        uint64_t has_next = 0;
        archive >> has_next;
        if (has_next != HAS_NEXT_FLAG) {
            return Result<void>();
        }

        std::string key;
        archive >> key;

        SCENE_DBG_LOG("Loading Component {}", key);

        auto it = p_scene.GetLibraryEntries().find(key);
        if (it == p_scene.GetLibraryEntries().end()) {
            return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "entry '{}' not found", key);
        }
        if (!it->second.m_manager->Serialize(archive, version)) {
            return CAVE_ERROR(ErrorCode::ERR_FILE_CORRUPT, "failed to serialize '{}'", key);
        }
    }
}
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif

void NameComponent::Serialize(Archive& p_archive, uint32_t p_version) {
    unused(p_version);

    p_archive.ArchiveValue(m_name);
}

void HierarchyComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(m_parent_id);
}

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

void CameraComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(m_flags);
    p_archive.ArchiveValue(m_near);
    p_archive.ArchiveValue(m_far);
    p_archive.ArchiveValue(m_fovy);
    p_archive.ArchiveValue(m_width);
    p_archive.ArchiveValue(m_height);
    p_archive.ArchiveValue(m_pitch);
    p_archive.ArchiveValue(m_yaw);
    p_archive.ArchiveValue(m_position);
    p_archive.ArchiveValue(m_ortho_height);
}

void LuaScriptComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(m_class_name);
    p_archive.ArchiveValue(m_path);
}

void LuaScriptComponent::OnDeserialized() {
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

void ClothComponent::Serialize(Archive& p_archive, uint32_t p_version) {
    CollisionObjectBase::Serialize(p_archive, p_version);

    CRASH_NOW_MSG("@TODO: implement");
}

void EnvironmentComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(sky.type);
    p_archive.ArchiveValue(sky.texturePath);
    p_archive.ArchiveValue(ambient.color);
}

void VoxelGiComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(flags);
}

#if 0
void AnimationComponent::Sampler::RegisterClass() {
    BEGIN_REGISTRY(AnimationComponent::Sampler);
    REGISTER_FIELD(AnimationComponent::Sampler, "key_frames.data", keyframeData);
    REGISTER_FIELD(AnimationComponent::Sampler, "key_frames.times", keyframeTimes);
    END_REGISTRY(AnimationComponent::Sampler);
}

void AnimationComponent::Channel::RegisterClass() {
    BEGIN_REGISTRY(AnimationComponent::Channel);
    REGISTER_FIELD_2(AnimationComponent::Channel, path);
    REGISTER_FIELD(AnimationComponent::Channel, "target_id", targetId);
    REGISTER_FIELD(AnimationComponent::Channel, "sampler_idx", samplerIndex);
    END_REGISTRY(AnimationComponent::Channel);
}

void AnimationComponent::RegisterClass() {
    BEGIN_REGISTRY(AnimationComponent);
    REGISTER_FIELD_2(AnimationComponent, flags);
    REGISTER_FIELD_2(AnimationComponent, start);
    REGISTER_FIELD_2(AnimationComponent, end);
    REGISTER_FIELD_2(AnimationComponent, timer);
    REGISTER_FIELD_2(AnimationComponent, amount);
    REGISTER_FIELD_2(AnimationComponent, speed);
    REGISTER_FIELD_2(AnimationComponent, channels);
    REGISTER_FIELD_2(AnimationComponent, samplers);
    END_REGISTRY(AnimationComponent);
}

void ArmatureComponent::RegisterClass() {
    BEGIN_REGISTRY(ArmatureComponent);
    REGISTER_FIELD(ArmatureComponent, "bone_collection", boneCollection, FieldFlag::BINARY);
    REGISTER_FIELD(ArmatureComponent, "inverse_matrices", inverseBindMatrices, FieldFlag::BINARY);
    END_REGISTRY(ArmatureComponent);
}

void MeshComponent::RegisterClass() {
    BEGIN_REGISTRY(MeshComponent);
    REGISTER_FIELD_2(MeshComponent, flags);
    REGISTER_FIELD_2(MeshComponent, subsets);
    REGISTER_FIELD(MeshComponent, "armature_id", armatureId);

    REGISTER_FIELD_2(MeshComponent, indices, FieldFlag::BINARY);
    REGISTER_FIELD_2(MeshComponent, positions, FieldFlag::BINARY);
    REGISTER_FIELD_2(MeshComponent, normals, FieldFlag::BINARY);
    REGISTER_FIELD_2(MeshComponent, tangents, FieldFlag::BINARY);
    REGISTER_FIELD_2(MeshComponent, texcoords_0, FieldFlag::BINARY);
    REGISTER_FIELD_2(MeshComponent, texcoords_1, FieldFlag::BINARY);
    REGISTER_FIELD_2(MeshComponent, joints_0, FieldFlag::BINARY);
    REGISTER_FIELD_2(MeshComponent, weights_0, FieldFlag::BINARY);
    REGISTER_FIELD_2(MeshComponent, color_0, FieldFlag::BINARY);
    END_REGISTRY(MeshComponent);
}

void LuaScriptComponent::RegisterClass() {
    BEGIN_REGISTRY(LuaScriptComponent);
    REGISTER_FIELD(LuaScriptComponent, "class_name", m_className);
    REGISTER_FIELD(LuaScriptComponent, "path", m_path);
    END_REGISTRY(LuaScriptComponent);
}

void ParticleEmitterComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(maxParticleCount);
    p_archive.ArchiveValue(particlesPerFrame);
    p_archive.ArchiveValue(particleScale);
    p_archive.ArchiveValue(particleLifeSpan);
    p_archive.ArchiveValue(startingVelocity);
    p_archive.ArchiveValue(gravity);
    p_archive.ArchiveValue(color);
    p_archive.ArchiveValue(texture);
}

void MeshEmitterComponent::Serialize(Archive& p_archive, uint32_t) {
    unused(p_archive);
    CRASH_NOW();
}

void MeshComponent::MeshSubset::RegisterClass() {
    BEGIN_REGISTRY(MeshComponent::MeshSubset);
    REGISTER_FIELD_2(MeshComponent::MeshSubset, material_id);
    REGISTER_FIELD_2(MeshComponent::MeshSubset, index_count);
    REGISTER_FIELD_2(MeshComponent::MeshSubset, index_offset);
    REGISTER_FIELD_2(MeshComponent::MeshSubset, local_bound);
    END_REGISTRY(MeshComponent::MeshSubset);
}

void RigidBodyComponent::RegisterClass() {
    BEGIN_REGISTRY(RigidBodyComponent);
    REGISTER_FIELD(RigidBodyComponent, "collision_type", collisionType);
    REGISTER_FIELD(RigidBodyComponent, "collision_mask", collisionMask);
    REGISTER_FIELD(RigidBodyComponent, "shape", shape);
    REGISTER_FIELD(RigidBodyComponent, "type", objectType);
    REGISTER_FIELD(RigidBodyComponent, "size", size);
    REGISTER_FIELD(RigidBodyComponent, "mass", mass);
    END_REGISTRY(RigidBodyComponent);
}

void ParticleEmitterComponent::RegisterClass() {
    BEGIN_REGISTRY(ParticleEmitterComponent);
    REGISTER_FIELD(ParticleEmitterComponent, "max", maxParticleCount);
    REGISTER_FIELD(ParticleEmitterComponent, "per_frame", particlesPerFrame);
    REGISTER_FIELD(ParticleEmitterComponent, "scale", particleScale);
    REGISTER_FIELD(ParticleEmitterComponent, "lifespan", particleLifeSpan);
    REGISTER_FIELD(ParticleEmitterComponent, "velocity", startingVelocity);
    REGISTER_FIELD(ParticleEmitterComponent, "gravity", gravity);
    REGISTER_FIELD(ParticleEmitterComponent, "color", color);
    REGISTER_FIELD(ParticleEmitterComponent, "texture", texture);
    END_REGISTRY(ParticleEmitterComponent);
}
void MeshEmitterComponent::RegisterClass() {
    BEGIN_REGISTRY(MeshEmitterComponent);
    // REGISTER_FIELD(MeshEmitterComponent, "texture", texture);
    END_REGISTRY(MeshEmitterComponent);
}

void ForceFieldComponent::RegisterClass() {
    BEGIN_REGISTRY(ForceFieldComponent);
    REGISTER_FIELD_2(ForceFieldComponent, strength);
    REGISTER_FIELD_2(ForceFieldComponent, radius);
    END_REGISTRY(ForceFieldComponent);
}
void ClothComponent::RegisterClass() {
    BEGIN_REGISTRY(ClothComponent);
    REGISTER_FIELD_2(ClothComponent, point_0);
    REGISTER_FIELD_2(ClothComponent, point_1);
    REGISTER_FIELD_2(ClothComponent, point_2);
    REGISTER_FIELD_2(ClothComponent, point_3);
    END_REGISTRY(ClothComponent);
}

void ForceFieldComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(strength);
    p_archive.ArchiveValue(radius);
}

void EnvironmentComponent::Sky::RegisterClass() {
    BEGIN_REGISTRY(EnvironmentComponent::Sky);
    REGISTER_FIELD(EnvironmentComponent::Sky, "type", type);
    REGISTER_FIELD(EnvironmentComponent::Sky, "texture", texturePath);
    END_REGISTRY(EnvironmentComponent::Sky);
}

void EnvironmentComponent::Ambient::RegisterClass() {
    BEGIN_REGISTRY(EnvironmentComponent::Ambient);
    REGISTER_FIELD_2(EnvironmentComponent::Ambient, color);
    END_REGISTRY(EnvironmentComponent::Ambient);
}

void VoxelGiComponent::RegisterClass() {
    BEGIN_REGISTRY(VoxelGiComponent);
    REGISTER_FIELD_2(VoxelGiComponent, flags);
    END_REGISTRY(VoxelGiComponent);
}
void EnvironmentComponent::RegisterClass() {
    BEGIN_REGISTRY(EnvironmentComponent);
    REGISTER_FIELD_2(EnvironmentComponent, sky);
    REGISTER_FIELD_2(EnvironmentComponent, ambient);
    END_REGISTRY(EnvironmentComponent);
}
#endif

}  // namespace cave
