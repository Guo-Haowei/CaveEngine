// =============================================================================
// File: public/cave/runtime/scene/SceneCommandExecutor.h
// =============================================================================
#pragma once
#include "cave/core/reflection/Meta.h"
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/scene/SceneChangeEvent.h"

namespace cave {

class Scene;
class SceneCommandBuffer;

class SceneCommandExecutor {
public:
    explicit SceneCommandExecutor(Scene& p_scene, ecs::ComponentRegistry& p_reg) noexcept;
    explicit SceneCommandExecutor(Scene& p_scene) noexcept;

    ecs::Entity CreateEntity();
    void RemoveEntity(ecs::Entity p_ent);

    void* AddComponent(ecs::Entity p_ent, ComponentId p_id);

    bool RemoveComponent(ecs::Entity p_ent, ComponentId p_id);

    bool ChangeProperty(ecs::Entity p_ent,
                        ComponentId p_cid,
                        const PropertyId& p_pid,
                        const void* p_data,
                        uint32_t p_data_size);

    ecs::Entity Resolve(ecs::Entity p_ent) const noexcept;

    void Playback(SceneCommandBuffer& p_cb);

private:
    void SetRemap(ecs::Entity p_temp, ecs::Entity p_real);

    Scene& m_scene;
    const ecs::ComponentRegistry& m_reg;

    std::vector<ecs::Entity> m_remap;
};

}  // namespace cave
