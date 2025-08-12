#pragma once
#include "engine/math/box.h"
#include "engine/runtime/physics_manager_interface.h"

struct b2WorldId;

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
    void AddStaticBox(const b2WorldId& p_world_id, const Box2& p_box);

    Option<uint32_t> m_world_id;
};

}  // namespace cave
