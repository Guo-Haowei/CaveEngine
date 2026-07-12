// =============================================================================
// File: cave/runtime/script/native/NativeScript.h
// =============================================================================
#pragma once
#include "cave/core/error/ErrorMacros.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/variant/Variant.h"
#include "cave/runtime/scene/SceneRuntime.h"

namespace cave {

enum class SceneSystemId : uint32_t;

class SceneCommandWriter;

class NativeScript {
public:
    virtual ~NativeScript() = default;

    virtual void alwaysRun(SceneCommandWriter&) {}
    virtual void start() {}
    virtual void destroy() {}

    virtual void update(float) {}

    virtual void onBodyEntered(ecs::Entity) {}
    virtual void onBodyStay(ecs::Entity) {}
    virtual void onBodyExited(ecs::Entity) {}

    ecs::Entity entity() const { return m_entity; }

    const VariantMap& params() const {
        return m_params;
    }

protected:
    SceneRuntime& runtime() { return *m_runtime; }

    SceneQuery& query() { return m_runtime->query(); }
    const SceneQuery& query() const { return m_runtime->query(); }

    template<typename T>
    T* system() { return m_runtime->system<T>(); }

    RuntimeServices& services() { return m_runtime->services(); }

    // @TODO: move to scene runtime?
    ViewId viewId() const { return m_runtime->viewId(); }
    ISceneTransitionRequests* transition() { return m_runtime->transition(); }

    template<typename T>
    T* component() { return query().component<T>(entity()); }
    template<typename T>
    const T* component() const { return query().component<T>(entity()); }

private:
    friend class NativeScriptSystem;

    void bind(SceneRuntime* runtime, ecs::Entity entity, const VariantMap& params);

    void unbind();

private:
    ecs::Entity m_entity{};
    VariantMap m_params{};
    SceneRuntime* m_runtime{};
};

}  // namespace cave