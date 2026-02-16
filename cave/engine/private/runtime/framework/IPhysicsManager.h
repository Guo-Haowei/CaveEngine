#pragma once
#include "cave/runtime/framework/IService.h"

namespace cave {

class Scene;

class IPhysicsManager : public IService,
                        public ServiceCreateRegistry<IPhysicsManager> {
public:
    using CreateFunc = IPhysicsManager* (*)();

    IPhysicsManager(std::string_view p_name)
        : IService(p_name) {}

    virtual void Update(Scene& p_scene, float p_timestep) = 0;

    virtual void OnSimBegin(Scene& p_scene) = 0;
    virtual void OnSimEnd() = 0;
};

}  // namespace cave
