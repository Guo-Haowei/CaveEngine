// =============================================================================
// File: cave/runtime/script/native/NativeScript.h
// =============================================================================
#pragma once
#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

struct SceneContext;

class NativeScript {
public:
    virtual ~NativeScript() = default;

    ecs::Entity entity() const { return entity_; }

    virtual void onCreate(SceneContext&) {}
    virtual void onDestroy() {}

    virtual void onUpdate(SceneContext&, float) {}

    virtual void onTriggerEnter(SceneContext&, ecs::Entity) {}
    virtual void onTriggerExit(SceneContext&, ecs::Entity) {}

private:
    friend class NativeScriptSystem;

    void bind(ecs::Entity entity) {
        entity_ = entity;
    }

    void unbind() {
        entity_ = ecs::Entity::Null();
    }

private:
    ecs::Entity entity_{};
};

}  // namespace cave