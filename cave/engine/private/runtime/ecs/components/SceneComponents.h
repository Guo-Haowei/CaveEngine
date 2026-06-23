#pragma once
#include "cave/core/math/AABB.h"
#include "cave/core/math/Angle.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {
#include "shader_defines.hlsl.h"
}  // namespace cave

namespace cave {

struct BvhAccel;
struct GpuMesh;
struct GpuStructuredBuffer;
struct ImageAsset;
class Archive;
class FileAccess;
class Scene;

struct NoSaveTag {
    CAVE_COMPONENT(NoSaveTag)
};

#pragma region COLLISION_OBJECT_COMPONENT

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
    math::Vec3f size;
    float mass{ 1.0f };

    RigidBodyComponent& InitCube(const math::Vec3f& p_half_size);

    RigidBodyComponent& InitSphere(float p_radius);

    RigidBodyComponent& InitGhost();

    void Serialize(Archive& p_archive, uint32_t p_version);
};

#pragma endregion COLLISION_OBJECT_COMPONENT

#if 0
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
        math::Vector4f color;
    } ambient;

    void Serialize(Archive& p_archive, uint32_t p_version);
};
#pragma endregion ENVIRONMENT_COMPONENT
#endif

// @TODO: move to particle system
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
        NONE = 0,
        RUNNING = BIT(0),
        RECYCLE = BIT(1),
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

}  // namespace cave
