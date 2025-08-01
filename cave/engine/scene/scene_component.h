#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/ecs/entity.h"
#include "engine/math/aabb.h"
#include "engine/math/angle.h"
#include "engine/reflection/reflection.h"

// @TODO: get rid of this
#include "scene_component_base.h"

#include "transform_component.h"

namespace cave {
#include "shader_defines.hlsl.h"
}  // namespace cave

namespace cave {

struct BvhAccel;
struct GpuMesh;
struct GpuStructuredBuffer;
struct ImageAsset;
struct TextAsset;
class Archive;
class FileAccess;
class Scene;

class NameComponent {
    CAVE_META(NameComponent)

    CAVE_PROP()
    std::string m_name;

public:
    NameComponent() = default;

    NameComponent(const char* p_name) { m_name = p_name; }

    void SetName(const char* p_name) { m_name = p_name; }
    void SetName(const std::string& p_name) { m_name = p_name; }

    const std::string& GetName() const { return m_name; }
    std::string& GetNameRef() { return m_name; }
};

class HierarchyComponent {
    CAVE_META(HierarchyComponent)

    CAVE_PROP()
    ecs::Entity m_parent_id;

    friend class Scene;

public:
    ecs::Entity GetParent() const { return m_parent_id; }
};

class VelocityComponent {
    CAVE_META(VelocityComponent)
public:
    CAVE_PROP(editor = Translation)
    Vector3f linear = Vector3f::Zero;
};

// @TODO: make it asset
#pragma region ANIMATION_COMPONENT
struct AnimationComponent {
    enum : uint32_t {
        NONE = 0,
        PLAYING = 1 << 0,
        LOOPED = 1 << 1,
    };

    struct Channel {
        enum Path {
            PATH_TRANSLATION,
            PATH_ROTATION,
            PATH_SCALE,

            PATH_UNKNOWN,
        };

        Path path = PATH_UNKNOWN;
        ecs::Entity targetId;
        int samplerIndex = -1;
    };

    struct Sampler {
        std::vector<float> keyframeTimes;
        std::vector<float> keyframeData;
    };

    bool IsPlaying() const { return flags & PLAYING; }
    bool IsLooped() const { return flags & LOOPED; }
    float GetLegnth() const { return end - start; }
    float IsEnd() const { return timer > end; }

    uint32_t flags = LOOPED;
    float start = 0;
    float end = 0;
    float timer = 0;
    float amount = 1;  // blend amount
    float speed = 1;

    std::vector<Channel> channels;
    std::vector<Sampler> samplers;

    void Serialize(Archive& p_archive, uint32_t p_version);
};
#pragma endregion ANIMATION_COMPONENT

// @TODO: make it asset
#pragma region ARMATURE_COMPONENT
struct ArmatureComponent {
    std::vector<ecs::Entity> boneCollection;
    std::vector<Matrix4x4f> inverseBindMatrices;

    // Non-Serialized
    std::vector<Matrix4x4f> boneTransforms;

    void Serialize(Archive& p_archive, uint32_t p_version);
};
#pragma endregion ARMATURE_COMPONENT

// @TODO: move the following to scripts
#pragma region COLLISION_OBJECT_COMPONENT

/*
    enum Type : uint32_t {
        PLAYER = BIT(1),
        FRIENDLY = BIT(2),
        HOSTILE = BIT(4),
    };

    player.collisionType = PLAYER;
    player.collisionMask = HOSTILE;

    hostile.collisionType = HOSTILE;
    hostile.collisionMask = PLAYER | FRIENDLY;
*/
struct CollisionObjectBase {
    uint32_t collisionType = 0;
    uint32_t collisionMask = 0;

    // Non-Serialized
    void* physicsObject{ nullptr };

    void Serialize(Archive& p_archive, uint32_t p_version);
};

struct RigidBodyComponent : CollisionObjectBase {
    enum CollisionShape : uint32_t {
        SHAPE_UNKNOWN,
        SHAPE_SPHERE,
        SHAPE_CUBE,
        SHAPE_MAX,
    };

    enum ObjectType : uint32_t {
        DYNAMIC,
        GHOST,
    };

    CollisionShape shape{ SHAPE_UNKNOWN };
    ObjectType objectType{ DYNAMIC };
    Vector3f size;
    float mass{ 1.0f };

    RigidBodyComponent& InitCube(const Vector3f& p_half_size);

    RigidBodyComponent& InitSphere(float p_radius);

    RigidBodyComponent& InitGhost();

    void Serialize(Archive& p_archive, uint32_t p_version);
};

#pragma endregion COLLISION_OBJECT_COMPONENT

#pragma region ENVIRONMENT_COMPONENT
struct EnvironmentComponent {
    enum Type : uint32_t {
        HDR_TEXTURE,
        PROCEDURE,
    };

    struct Sky {
        Type type;
        std::string texturePath;
        // Non-Serialized
        mutable const ImageAsset* textureAsset;
    } sky;

    struct Ambient {
        Vector4f color;
    } ambient;

    void Serialize(Archive& p_archive, uint32_t p_version);
};

struct VoxelGiComponent {
    enum : uint32_t {
        ENABLED = BIT(1),
        SHOW_DEBUG_BOX = BIT(2),
    };

    uint32_t flags = 0;
    // Non-serialized
    AABB region;

    bool Enabled() const { return flags & ENABLED; }
    bool ShowDebugBox() const { return flags & SHOW_DEBUG_BOX; }

    void Serialize(Archive& p_archive, uint32_t p_version);
};
#pragma endregion ENVIRONMENT_COMPONENT

#pragma region PARTICLE_EMITTER_COMPONENT
#if 0
struct ParticleEmitterComponent {
    bool gravity{ false };  // @TODO: force instead of gravity
    int maxParticleCount{ 1000 };
    int particlesPerFrame{ 10 };
    float particleScale{ 1.0f };
    float particleLifeSpan{ 3.0f };
    Vector3f startingVelocity{ 0.0f };
    Vector4f color{ Vector4f::One };
    std::string texture;

    // Non-Serialized
    std::shared_ptr<GpuStructuredBuffer> particleBuffer{ nullptr };
    std::shared_ptr<GpuStructuredBuffer> counterBuffer{ nullptr };
    std::shared_ptr<GpuStructuredBuffer> deadBuffer{ nullptr };
    std::shared_ptr<GpuStructuredBuffer> aliveBuffer[2]{ nullptr, nullptr };

    uint32_t aliveBufferIndex{ 0 };

    uint32_t GetPreIndex() const { return aliveBufferIndex; }
    uint32_t GetPostIndex() const { return 1 - aliveBufferIndex; }

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
};
#endif
#pragma endregion PARTICLE_EMITTER_COMPONENT

#pragma region MESH_EMITTER_COMPONENT
#if 0
struct MeshEmitterComponent {
    enum : uint32_t {
        NONE = BIT(0),
        RUNNING = BIT(1),
        RECYCLE = BIT(2),
    };

    struct Particle {
        Vector3f position;
        float lifespan;
        Vector3f rotation;
        float scale;
        Vector3f velocity;
        Vector3f angularVelocity;

        void Init(float p_lifespan,
                  const Vector3f& p_position,
                  const Vector3f& p_velocity,
                  const Vector3f& p_rotation,
                  float p_scale) {
            position = p_position;
            lifespan = p_lifespan;
            velocity = p_velocity;
            rotation = p_rotation;
            scale = p_scale;
        }
    };

    uint32_t flags{ NONE };
    int maxMeshCount{ 128 };
    int emissionPerFrame{ 1 };
    ecs::Entity meshId;
    Vector3f gravity{ 0 };
    float scale{ 1.0f };
    Vector2f vxRange{ 0 };
    Vector2f vyRange{ 0 };
    Vector2f vzRange{ 0 };
    Vector2f lifetimeRange{ 3, 3 };

    // Non Serialized
    std::vector<Particle> particles;
    // use this to avoid feeding wrong index
    struct Index {
        uint32_t v;
    };
    std::vector<Index> deadList;
    std::vector<Index> aliveList;

    bool IsRunning() const { return flags & RUNNING; }
    bool IsRecycle() const { return flags & RECYCLE; }
    void Start() { flags |= RUNNING; }
    void Stop() { flags &= ~RUNNING; }

    void UpdateParticle(Index p_index, float p_timestep);
    void Reset();

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() { Reset(); }
};
#endif
#pragma endregion MESH_EMITTER_COMPONENT

#pragma region FORCE_FIELD_COMPONENT
#if 0
struct ForceFieldComponent {
    float strength{ 1.0f };
    float radius{ 0.01f };

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
};
#endif
#pragma endregion FORCE_FIELD_COMPONENT

// #pragma region _COMPONENT
// #pragma endregion _COMPONENT

}  // namespace cave
