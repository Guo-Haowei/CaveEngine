#include "Engine.h"

#include "cave/runtime/ecs/ComponentRegistry.h"

#include "engine/private/core/os/os.h"
#include "engine/private/core/os/threads.h"
#include "engine/private/systems/job_system/job_system.h"

// @TODO: refactor
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

static void RegisterBuiltinComponents() {
    auto& reg = s_component_reg;

#define REGISTER_COMPONENT(T, ...)              \
    reg.Register({                              \
        .id = T##_Id,                           \
        .name = #T,                             \
        .name_id = StringId(#T),                \
        .size = sizeof(T),                      \
        .align = alignof(T),                    \
        .version = 0,                           \
        .props = MetaDataTable<T>::GetFields(), \
    });

    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT
}

ecs::ComponentRegistry& GetComponentRegistry() {
    return s_component_reg;
}

}  // namespace cave::engine
