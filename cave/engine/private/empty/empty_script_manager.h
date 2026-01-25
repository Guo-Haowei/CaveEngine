#pragma once
#include "engine/private/runtime/framework/ScriptManager.h"

namespace cave {

class EmptyScriptManager : public IScriptManager {
public:
    EmptyScriptManager()
        : IScriptManager("EmptyScriptManager") {}

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
