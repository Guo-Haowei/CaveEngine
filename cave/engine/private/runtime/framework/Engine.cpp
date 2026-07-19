#include "Engine.h"

#include "cave/core/threading/JobSystem.h"
#include "cave/core/threading/Threads.h"
#include "cave/core/reflection/MetaRegistry.h"

#include "engine/private/core/os/os.h"

namespace cave::engine {

static OS* s_os;
static MetaRegistry s_meta_reg;

bool InitializeCore() {
    if (s_os) {
        return true;
    }

    s_os = new OS;
    s_os->Initialize();

    MetaRegistry::builtin(s_meta_reg);

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

const MetaRegistry& GetMetaRegistry() {
    return s_meta_reg;
}

}  // namespace cave::engine
