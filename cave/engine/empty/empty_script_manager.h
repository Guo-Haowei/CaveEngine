#pragma once
#include "engine/runtime/script_manager.h"

namespace cave {

class NullScriptManager : public IScriptManager {
public:
    NullScriptManager()
        : IScriptManager("NullScriptManager") {}

    virtual void OnSimBegin(Scene&) {}
    virtual void OnSimEnd() {}

    virtual void Update(Scene&, float) {}
    virtual void OnCollision(Scene&, ecs::Entity, ecs::Entity) {}

protected:
    virtual Result<void> InitializeImpl() {
        return Result<void>();
    }

    virtual void FinalizeImpl() {}
};

}  // namespace cave
