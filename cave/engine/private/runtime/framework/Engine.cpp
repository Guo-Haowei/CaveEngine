#include "Engine.h"

#include "cave/runtime/ecs/ComponentRegistry.h"

#include "engine/private/core/os/os.h"
#include "engine/private/core/os/threads.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/systems/job_system/job_system.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave::engine {

static OS* s_os;
static ecs::ComponentRegistry s_component_reg;

static void RegisterBuiltinComponents();

bool InitializeCore() {
    if (s_os) {
        return true;
    }

    s_os = new OS;
    s_os->Initialize();

    RegisterBuiltinComponents();

    thread::Initialize();
    jobsystem::Initialize();

    return true;
}

void FinalizeCore() {
    if (!s_os) {
        return;
    }

    jobsystem::Finalize();
    thread::Finailize();

    s_os->Finalize();
    delete s_os;
    s_os = nullptr;
}

static void Transform_OnEdited(Scene& p_scene,
                               ecs::Entity p_ent,
                               ComponentId,
                               const PropertyId&,
                               const void*,
                               uint32_t) {
    auto* t = (TransformComponent*)p_scene.Storage().GetRaw(p_ent, TransformComponent_Id);
    if (DEV_VERIFY(t)) {
        t->SetDirty();
    }
}

static void MeshRenderer_OnEdited(Scene& p_scene,
                                  ecs::Entity p_ent,
                                  ComponentId,
                                  const PropertyId& p_prop_id,
                                  const void*,
                                  uint32_t) {
    if (p_prop_id == StringId("mesh_id")) {
        auto* mesh = (MeshRendererComponent*)p_scene.Storage().GetRaw(p_ent, MeshRendererComponent_Id);
        if (DEV_VERIFY(mesh)) {
            mesh->OnDeserialized();
        }
    }
}

static void Materail_OnEdited(Scene& p_scene,
                              ecs::Entity p_ent,
                              ComponentId,
                              const PropertyId& p_prop_id,
                              const void*,
                              uint32_t) {
    if (p_prop_id == StringId("material_id")) {
        auto* m = (MaterialComponent*)p_scene.Storage().GetRaw(p_ent, MaterialComponent_Id);
        if (DEV_VERIFY(m)) {
            m->OnDeserialized();
        }
    }
}

static void RegisterBuiltinComponents() {
    auto& reg = s_component_reg;

#define REGISTER_COMPONENT(T, ...)              \
    reg.Register({                              \
        .id = T##_Id,                           \
        .name = #T,                             \
        .size = sizeof(T),                      \
        .align = alignof(T),                    \
        .version = 0,                           \
        .props = MetaDataTable<T>::GetFields(), \
    });

    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

    {
        ecs::ComponentMeta& meta = reg.GetMut(TransformComponent_Id);
        meta.on_edited = Transform_OnEdited;
    }
    {
        ecs::ComponentMeta& meta = reg.GetMut(MeshRendererComponent_Id);
        meta.on_edited = MeshRenderer_OnEdited;
    }
    {
        ecs::ComponentMeta& meta = reg.GetMut(MaterialComponent_Id);
        meta.on_edited = Materail_OnEdited;
    }
}

ecs::ComponentRegistry& GetComponentRegistry() {
    return s_component_reg;
}

}  // namespace cave::engine
