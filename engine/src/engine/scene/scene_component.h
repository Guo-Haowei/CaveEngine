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
class ScriptableEntity;

#pragma region NAME_COMPONENT
class NameComponent {
    CAVE_META(NameComponent)

    CAVE_PROP(type = name)
    std::string m_name;

public:
    NameComponent() = default;

    NameComponent(const char* p_name) { m_name = p_name; }

    void SetName(const char* p_name) { m_name = p_name; }
    void SetName(const std::string& p_name) { m_name = p_name; }

    const std::string& GetName() const { return m_name; }
    std::string& GetNameRef() { return m_name; }

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
};
#pragma endregion NAME_COMPONENT

#pragma region HIERARCHY_COMPONENT
class HierarchyComponent {
    CAVE_META(HierarchyComponent)

    CAVE_PROP(type = id)
    ecs::Entity m_parent_id;

    friend class Scene;

public:
    ecs::Entity GetParent() const { return m_parent_id; }

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
};
#pragma endregion HIERARCHY_COMPONENT

// @TODO: make it asset
#pragma region MESH_COMPONENT
enum class VertexAttributeName : uint8_t {
    POSITION = 0,
    NORMAL,
    TEXCOORD_0,
    TEXCOORD_1,
    TANGENT,
    JOINTS_0,
    WEIGHTS_0,
    COLOR_0,
    COUNT,
};

struct MeshComponent {
    enum : uint32_t {
        NONE = BIT(0),
        RENDERABLE = BIT(1),
        DOUBLE_SIDED = BIT(2),
        DYNAMIC = BIT(3),
    };

    struct VertexAttribute {
        VertexAttributeName attribName;
        uint32_t offsetInByte{ 0 };
        uint32_t strideInByte{ 0 };

        uint32_t elementCount{ 0 };
    };

    uint32_t flags = RENDERABLE;
    std::vector<uint32_t> indices;
    std::vector<Vector3f> positions;
    std::vector<Vector3f> normals;
    std::vector<Vector3f> tangents;
    std::vector<Vector2f> texcoords_0;
    std::vector<Vector2f> texcoords_1;
    std::vector<Vector4i> joints_0;
    std::vector<Vector4f> weights_0;
    std::vector<Vector4f> color_0;

    struct MeshSubset {
        Guid material_id;
        uint32_t index_offset = 0;
        uint32_t index_count = 0;
        AABB local_bound;

        // Non-serialized
        Handle<MaterialAsset> mat_handle;
    };
    std::vector<MeshSubset> subsets;

    ecs::Entity armatureId;

    // Non-serialized
    mutable std::shared_ptr<GpuMesh> gpuResource;
    mutable std::shared_ptr<BvhAccel> bvh;
    AABB localBound;

    mutable std::vector<Vector3f> updatePositions;
    mutable std::vector<Vector3f> updateNormals;

    VertexAttribute attributes[std::to_underlying(VertexAttributeName::COUNT)];

    void CreateRenderData();

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized();
};
#pragma endregion MESH_COMPONENT

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
    void OnDeserialized() {}
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
    void OnDeserialized() {}
};
#pragma endregion ARMATURE_COMPONENT

#pragma region MESH_RENDERER_COMPONENT
struct MeshRenderer : public ComponentFlagBase {
    enum : uint32_t {
        FLAG_RENDERABLE = BIT(1),
        FLAG_CAST_SHADOW = BIT(2),
        FLAG_TRANSPARENT = BIT(3),
    };

    ecs::Entity meshId;

    MeshRenderer() {
        flags |= FLAG_RENDERABLE | FLAG_CAST_SHADOW;
    }

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
};
#pragma endregion MESH_RENDERER_COMPONENT

#pragma region LUA_SCRIPT_COMPONENT
class LuaScriptComponent {
public:
    LuaScriptComponent& SetClassName(std::string_view p_class_name);
    LuaScriptComponent& SetPath(std::string_view p_path);

    const std::string& GetPath() const { return m_path; }
    const std::string& GetClassName() const { return m_className; }
    int GetInstance() const { return m_instance; }

    std::string& GetPathRef() { return m_path; }
    std::string& GetClassNameRef() { return m_className; }

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized();

private:
    std::string m_className;
    std::string m_path;

    // Non-Serialized
    int m_instance{ 0 };

    friend class LuaScriptManager;
};
#pragma endregion LUA_SCRIPT_COMPONENT

#pragma region NATIVE_SCRIPT_COMPONENT
struct NativeScriptComponent {
    using InstantiateFunc = ScriptableEntity* (*)(void);
    using DestroyFunc = void (*)(NativeScriptComponent*);

    std::string scriptName = typeid(this).name();
    ScriptableEntity* instance{ nullptr };
    InstantiateFunc instantiateFunc{ nullptr };
    DestroyFunc destroyFunc{ nullptr };

    NativeScriptComponent() = default;

    ~NativeScriptComponent();

    NativeScriptComponent(const NativeScriptComponent& p_rhs);

    NativeScriptComponent& operator=(const NativeScriptComponent& p_rhs);

    template<typename T>
    NativeScriptComponent& Bind() {
        instantiateFunc = []() -> ScriptableEntity* {
            return new T();
        };
        destroyFunc = [](NativeScriptComponent* p_script) {
            delete (T*)p_script->instance;
            p_script->instance = nullptr;
        };

        return *this;
    }

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
};
#pragma endregion NATIVE_SCRIPT_COMPONENT

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
    void OnDeserialized() {}
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
    void OnDeserialized() {}
};

enum ClothFixFlag : uint32_t {
    CLOTH_FIX_0 = BIT(1),
    CLOTH_FIX_1 = BIT(2),
    CLOTH_FIX_2 = BIT(3),
    CLOTH_FIX_3 = BIT(4),

    CLOTH_FIX_ALL = CLOTH_FIX_0 | CLOTH_FIX_1 | CLOTH_FIX_2 | CLOTH_FIX_3,
};
DEFINE_ENUM_BITWISE_OPERATIONS(ClothFixFlag);

struct ClothComponent : CollisionObjectBase {
    Vector3f point_0;
    Vector3f point_1;
    Vector3f point_2;
    Vector3f point_3;
    Vector2i res;
    ClothFixFlag fixedFlags;

    // Non-Serialized
    void* physicsObject{ nullptr };

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
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
    void OnDeserialized() {}
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
    void OnDeserialized() {}
};
#pragma endregion ENVIRONMENT_COMPONENT

#pragma region PARTICLE_EMITTER_COMPONENT
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
#pragma endregion PARTICLE_EMITTER_COMPONENT

#pragma region MESH_EMITTER_COMPONENT
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
#pragma endregion MESH_EMITTER_COMPONENT

#pragma region FORCE_FIELD_COMPONENT
struct ForceFieldComponent {
    float strength{ 1.0f };
    float radius{ 0.01f };

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized() {}
};
#pragma endregion FORCE_FIELD_COMPONENT

#pragma region LIGHT_COMPONENT
class LightComponent {
public:
    enum : uint32_t {
        NONE = BIT(0),
        DIRTY = BIT(1),
        CAST_SHADOW = BIT(2),
        SHADOW_REGION = BIT(3),
    };

    bool IsDirty() const { return m_flags & DIRTY; }
    void SetDirty(bool p_dirty = true) { p_dirty ? m_flags |= DIRTY : m_flags &= ~DIRTY; }

    bool CastShadow() const { return m_flags & CAST_SHADOW; }
    void SetCastShadow(bool p_cast = true) { p_cast ? m_flags |= CAST_SHADOW : m_flags &= ~CAST_SHADOW; }

    bool HasShadowRegion() const { return m_flags & SHADOW_REGION; }
    void SetShadowRegion(bool p_region = true) { p_region ? m_flags |= SHADOW_REGION : m_flags &= ~SHADOW_REGION; }

    int GetType() const { return m_type; }
    void SetType(int p_type) { m_type = p_type; }

    float GetMaxDistance() const { return m_maxDistance; }
    int GetShadowMapIndex() const { return m_shadowMapIndex; }

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized();

    const auto& GetMatrices() const { return m_lightSpaceMatrices; }
    const Vector3f& GetPosition() const { return m_position; }

    struct Attenuation {
        float constant;
        float linear;
        float quadratic;
    } m_atten;

    AABB m_shadowRegion;

    uint32_t m_flags = DIRTY;
    int m_type = LIGHT_TYPE_INFINITE;

    Guid m_material_id;

    // Non-serialized
    float m_maxDistance;
    Vector3f m_position;
    int m_shadowMapIndex = -1;
    std::array<Matrix4x4f, 6> m_lightSpaceMatrices;
    Handle<MaterialAsset> m_material_handle;
};
#pragma endregion LIGHT_COMPONENT

// #pragma region _COMPONENT
// #pragma endregion _COMPONENT

}  // namespace cave
