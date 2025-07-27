#pragma once
#include "engine/runtime/physics_manager_interface.h"

namespace cave {

class NullPhysicsManager : public IPhysicsManager {
public:
    NullPhysicsManager()
        : IPhysicsManager("NullPhysicsManager") {}

    auto InitializeImpl() -> Result<void> override { return Result<void>(); }
    void FinalizeImpl() override {}

    void Update(Scene&, float) override {}

    void OnSimBegin(Scene&) override {}
    void OnSimEnd() override {}
};

}  // namespace cave
