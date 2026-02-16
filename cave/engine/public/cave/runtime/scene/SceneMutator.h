// =============================================================================
// File: public/cave/runtime/scene/SceneMutator.h
// =============================================================================
#pragma once
#include "cave/core/reflection/Meta.h"
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/scene/SceneChangeEvent.h"

namespace cave {

class Scene;

class SceneMutator {
public:
    explicit SceneMutator(Scene& p_scene, ecs::ComponentRegistry& p_reg) noexcept;
    explicit SceneMutator(Scene& p_scene) noexcept;

    ecs::Entity CreateEntity();
    void RemoveEntity(ecs::Entity p_ent);

    void* AddComponent(ecs::Entity p_ent, ComponentId p_id);

    bool RemoveComponent(ecs::Entity p_ent, ComponentId p_id);

    bool ChangeProperty(ecs::Entity p_ent,
                        ComponentId p_cid,
                        const PropertyId& p_pid,
                        const void* p_data,
                        uint32_t p_data_size,
                        void* p_old_data = nullptr);

    Scene& GetScene() { return m_scene; }

private:
    Scene& m_scene;
    const ecs::ComponentRegistry& m_reg;
};

}  // namespace cave
