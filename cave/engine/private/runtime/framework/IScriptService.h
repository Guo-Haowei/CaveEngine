#pragma once
#include "cave/core/ids/Entity.h"
#include "engine/private/runtime/framework/Module.h"

namespace cave {

class Scene;

class IScriptService : public Module,
                       public ModuleCreateRegistry<IScriptService> {
public:
    IScriptService(std::string_view p_name)
        : Module(p_name) {}

    virtual void OnSimBegin(Scene& p_scene) = 0;
    virtual void OnSimEnd() = 0;

    virtual void Update(Scene& p_scene, float p_timestep) = 0;
    virtual void OnCollision(Scene& p_scene, ecs::Entity p_ent_1, ecs::Entity p_ent_2) = 0;
};

}  // namespace cave
