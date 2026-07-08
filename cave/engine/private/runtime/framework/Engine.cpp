#include "Engine.h"

#include "cave/core/threading/JobSystem.h"
#include "cave/core/threading/Threads.h"
#include "cave/runtime/ecs/ComponentRegistry.h"

#include "engine/private/core/os/os.h"

namespace cave::engine {

static OS* s_os;
static ecs::ComponentRegistry s_component_reg;

bool InitializeCore() {
    if (s_os) {
        return true;
    }

    s_os = new OS;
    s_os->Initialize();

    ecs::ComponentRegistry::builtin(s_component_reg);

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
