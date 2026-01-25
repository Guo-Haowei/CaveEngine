#pragma once
#include "engine/runtime/framework/IPhysicsManager.h"

namespace cave {

class EmptyPhysicsManager : public IPhysicsManager {
public:
    EmptyPhysicsManager()
        : IPhysicsManager("EmptyPhysicsManager") {}

    auto InitializeImpl() -> Result<void> override { return Result<void>(); }
    void FinalizeImpl() override {}

    void Update(Scene&, float) override {}

    void OnSimBegin(Scene&) override {}
    void OnSimEnd() override {}
};

}  // namespace cave
