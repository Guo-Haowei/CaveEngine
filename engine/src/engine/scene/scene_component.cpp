#include "scene_component.h"

#include "engine/core/io/archive.h"
#include "engine/math/matrix_transform.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/graphics_manager_interface.h"

namespace cave {

#pragma region LUA_SCRIPT_COMPONENT
LuaScriptComponent& LuaScriptComponent::SetClassName(std::string_view p_class_name) {
    if (DEV_VERIFY(!p_class_name.empty())) {
        m_className = p_class_name;
    }

    return *this;
}

LuaScriptComponent& LuaScriptComponent::SetPath(std::string_view p_path) {
    if (p_path != m_path) {
        // LOG_VERBOSE("changing script '{}' to '{}'", m_path, p_path);
        m_path = p_path;
    }

    return *this;
}
#pragma endregion LUA_SCRIPT_COMPONENT

#pragma region NATIVE_SCRIPT_COMPONENT
NativeScriptComponent::~NativeScriptComponent() {
    // DON'T DELETE instance, because it could be copied from other script
    // Memory leak!!!
}

NativeScriptComponent::NativeScriptComponent(const NativeScriptComponent& p_rhs) {
    *this = p_rhs;
}

NativeScriptComponent& NativeScriptComponent::operator=(const NativeScriptComponent& p_rhs) {
    instantiateFunc = p_rhs.instantiateFunc;
    destroyFunc = p_rhs.destroyFunc;
    instance = p_rhs.instance;
    return *this;
}
#pragma endregion NATIVE_SCRIPT_COMPONENT

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

#pragma region MESH_EMITTER_COMPONENT
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

#pragma endregion MESH_EMITTER_COMPONENT

#pragma region SOFT_BODY_COMPONENT
#pragma endregion SOFT_BODY_COMPONENT

#pragma region ENVIRONMENT_COMPONENT
#pragma endregion ENVIRONMENT_COMPONENT

}  // namespace cave
