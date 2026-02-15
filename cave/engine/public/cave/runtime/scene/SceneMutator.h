#pragma once
#include "cave/core/reflection/Meta.h"
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/scene/SceneChangeEvent.h"

namespace cave {

class Scene;

class SceneMutator {
public:
    explicit SceneMutator(Scene& p_scene) noexcept;

    bool ModifyField(ecs::Entity p_ent,
                     ComponentId p_comp_id,
                     PropertyId p_property,
                     const void* p_data,
                     uint32_t p_data_size,
                     void* p_old_data = nullptr);

private:
    Scene& m_scene;
    const ecs::ComponentRegistry& m_reg;
};

}  // namespace cave
