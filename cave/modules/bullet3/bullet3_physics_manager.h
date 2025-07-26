#pragma once
#include "engine/runtime/physics_manager_interface.h"

namespace cave {

class Scene;

class Bullet3PhysicsManager : public IPhysicsManager {
public:
    Bullet3PhysicsManager()
        : IPhysicsManager("Bullet3PhysicsManager") {}

    void Update(Scene& p_scene, float p_timestep) override;

    void OnSimBegin(Scene& p_scene) override;
    void OnSimEnd() override;

protected:
    Result<void> InitializeImpl() override;
    void FinalizeImpl() override;

    void UpdateCollision(Scene& p_scene);
    void UpdateSimulation(Scene& p_scene, float p_timestep);
};

}  // namespace cave
