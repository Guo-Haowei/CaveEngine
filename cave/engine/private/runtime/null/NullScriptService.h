#pragma once
#include "engine/private/runtime/framework/IScriptService.h"

namespace cave {

class NullScriptService : public IScriptService {
public:
    NullScriptService()
        : IScriptService("NullScriptService") {}

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
