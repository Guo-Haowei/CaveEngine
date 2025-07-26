#pragma once
#include "engine/runtime/physics_manager_interface.h"

namespace cave {

class Box2dPhysicsManager : public IPhysicsManager {
public:
    Box2dPhysicsManager()
        : IPhysicsManager("Box2dPhysicsManager") {}

    void Update(Scene& p_scene, float p_timestep) override;

    void OnSimBegin(Scene& p_scene) override;
    void OnSimEnd() override;

protected:
    Result<void> InitializeImpl() override;
    void FinalizeImpl() override;

    Option<uint32_t> m_world_id;
};

}  // namespace cave
