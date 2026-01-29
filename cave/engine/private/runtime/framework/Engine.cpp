#include "Engine.h"

#include "engine/private/core/os/os.h"
#include "engine/private/core/os/threads.h"
#include "engine/private/systems/job_system/job_system.h"

namespace cave {

static OS* s_os;

bool engine::InitializeCore() {
    if (s_os) {
        return true;
    }

    s_os = new OS;
    s_os->Initialize();

    thread::Initialize();
    jobsystem::Initialize();

    return true;
}

void engine::FinalizeCore() {
    if (!s_os) {
        return;
    }

    jobsystem::Finalize();
    thread::Finailize();

    s_os->Finalize();
    delete s_os;
    s_os = nullptr;
}

}  // namespace cave
