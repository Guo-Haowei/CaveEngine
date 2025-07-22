#pragma once
#include "engine/ecs/entity.h"
#include "engine/runtime/module.h"

namespace cave {

class Scene;

class IScriptManager : public Module,
                       public ModuleCreateRegistry<IScriptManager> {
public:
    IScriptManager(std::string_view p_name)
        : Module(p_name) {}

    virtual void OnSimBegin(Scene& p_scene) = 0;
    virtual void OnSimEnd(Scene& p_scene) = 0;

    virtual void Update(Scene& p_scene, float p_timestep) = 0;
    virtual void OnCollision(Scene& p_scene, ecs::Entity p_entity_1, ecs::Entity p_entity_2) = 0;
};

}  // namespace cave
