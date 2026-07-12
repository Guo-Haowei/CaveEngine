// =============================================================================
// File: cave/runtime/script/native/NativeScript.h
// =============================================================================
#pragma once
#include "cave/core/error/ErrorMacros.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/variant/Variant.h"
#include "cave/runtime/scene/SceneContext.h"

namespace cave {

enum class SceneSystemId : uint32_t;

class SceneCommandWriter;

class NativeScript {
public:
    virtual ~NativeScript() = default;

    virtual void alwaysRun(SceneContext&, SceneCommandWriter&) {}
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
    SceneQuery& query() { return m_context->query; }
    const SceneQuery& query() const { return m_context->query; }

    template<typename T>
    T* system() { return reinterpret_cast<T*>(system(T::kSystemId)); }

    RuntimeServices& services() { return m_context->services; }
    SceneRuntime& runtime() { return m_context->runtime; }

    // @TODO: move to scene runtime?
    ViewId viewId() const { return m_context->view_id; }
    ISceneTransitionRequests* sceneTransition() { return m_context->scene_transition; }

    SceneContext& context() { return *m_context; }

    template<typename T>
    T* component() { return query().component<T>(entity()); }
    template<typename T>
    const T* component() const { return query().component<T>(entity()); }

private:
    friend class NativeScriptSystem;

    void bind(SceneContext& ctx, ecs::Entity entity, const VariantMap& params);

    void unbind();

private:
    void* system(SceneSystemId system_id);

    ecs::Entity m_entity{};
    VariantMap m_params{};
    SceneContext* m_context{};
};

}  // namespace cave