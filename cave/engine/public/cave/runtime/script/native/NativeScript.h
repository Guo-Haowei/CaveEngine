// =============================================================================
// File: cave/runtime/script/native/NativeScript.h
// =============================================================================
#pragma once
#include "cave/core/error/ErrorMacros.h"
#include "cave/core/variant/Variant.h"
#include "cave/core/ids/Entity.h"

namespace cave {

struct SceneContext;
class SceneCommandWriter;

class NativeScript {
public:
    virtual ~NativeScript() = default;

    virtual void alwaysRun(SceneContext&, SceneCommandWriter&) {}
    virtual void start(SceneContext&) {}
    virtual void destroy() {}

    virtual void update(SceneContext&, float) {}

    virtual void onBodyEntered(SceneContext&, ecs::Entity) {}
    virtual void onBodyStay(SceneContext&, ecs::Entity) {}
    virtual void onBodyExited(SceneContext&, ecs::Entity) {}

    ecs::Entity entity() const { return m_entity; }

    const VariantMap& params() const {
        return m_params;
    }

private:
    friend class NativeScriptSystem;

    void bind(ecs::Entity entity, const VariantMap& params) {
        m_entity = entity;
        m_params = params;
    }

    void unbind() {
        m_entity = ecs::Entity::null();
        m_params.clear();
    }

private:
    ecs::Entity m_entity{};
    VariantMap m_params{};
};

}  // namespace cave