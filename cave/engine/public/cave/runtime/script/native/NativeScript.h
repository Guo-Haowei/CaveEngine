// =============================================================================
// File: cave/runtime/script/native/NativeScript.h
// =============================================================================
#pragma once
#include "cave/core/error/ErrorMacros.h"
#include "cave/core/variant/Variant.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

struct SceneContext;

class NativeScript {
public:
    virtual ~NativeScript() = default;

    virtual void onCreate(SceneContext&) {}
    virtual void onDestroy() {}

    virtual void onUpdate(SceneContext&, float) {}

    virtual void onBodyEntered(SceneContext&, ecs::Entity) {}
    virtual void onBodyOverlapping(SceneContext&, ecs::Entity) {}
    virtual void onBodyExited(SceneContext&, ecs::Entity) {}

    ecs::Entity entity() const { return entity_; }

    const VariantMap& params() const {
        return params_;
    }

private:
    friend class NativeScriptSystem;

    void bind(ecs::Entity entity, const VariantMap& params) {
        entity_ = entity;
        params_ = params;
    }

    void unbind() {
        entity_ = ecs::Entity::Null();
        params_.clear();
    }

private:
    ecs::Entity entity_{};
    VariantMap params_{};
};

}  // namespace cave