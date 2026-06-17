// =============================================================================
// File: cave/runtime/script/native/NativeScript.h
// =============================================================================
#pragma once
#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

class SceneContext;

class NativeScript {
public:
    virtual ~NativeScript() = default;

    ecs::Entity entity() const {
        return entity_;
    }

protected:
    SceneContext& context() {
        DEV_ASSERT(context_);
        return *context_;
    }

    const SceneContext& context() const {
        DEV_ASSERT(context_);
        return *context_;
    }

    virtual void onCreate() {}
    virtual void onDestroy() {}

    virtual void onUpdate(float) {}

private:
    friend class NativeScriptSystem;

    void bind(SceneContext& ctx, ecs::Entity entity) {
        context_ = &ctx;
        entity_ = entity;
    }

    void unbind() {
        context_ = nullptr;
        entity_ = ecs::Entity::Null();
    }

private:
    SceneContext* context_{ nullptr };

    ecs::Entity entity_{};
};

}  // namespace cave