#include "Engine.h"

#include "cave/runtime/ecs/ComponentRegistry.h"

#include "engine/private/core/os/os.h"
#include "engine/private/core/os/threads.h"
#include "engine/private/systems/job_system/job_system.h"

namespace cave::engine {

static OS* s_os;
static ecs::ComponentRegistry s_component_reg;

bool InitializeCore() {
    if (s_os) {
        return true;
    }

    s_os = new OS;
    s_os->Initialize();

    ecs::ComponentRegistry::Builtin(s_component_reg);

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

ecs::ComponentRegistry& GetComponentRegistry() {
    return s_component_reg;
}

}  // namespace cave::engine
