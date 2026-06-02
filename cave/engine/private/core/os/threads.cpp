#include "threads.h"

#include <latch>
#include <thread>

#include "cave/core/diagnostics/Profiler.h"
#include "engine/private/drivers/windows/win32_prerequisites.h"

namespace cave::thread {

using ThreadMainFunc = void (*)();

struct ThreadObject {
    const char* name;
    ThreadMainFunc threadFunc;
    uint32_t id{ 0 };
    std::thread threadObject{};
};

static thread_local uint32_t g_thread_id;

static struct {
    std::atomic_bool shutdownRequested;
    std::array<ThreadObject, THREAD_MAX> threads = {
        ThreadObject{ "THREAD_MAIN", []() {} },
#if USING(ENABLE_JOB_SYSTEM)
        ThreadObject{ "THREAD_JOBSYSTEM_WORKER_1", jobsystem::WorkerMain },
        ThreadObject{ "THREAD_JOBSYSTEM_WORKER_2", jobsystem::WorkerMain },
        ThreadObject{ "THREAD_JOBSYSTEM_WORKER_3", jobsystem::WorkerMain },
        ThreadObject{ "THREAD_JOBSYSTEM_WORKER_4", jobsystem::WorkerMain },
        ThreadObject{ "THREAD_JOBSYSTEM_WORKER_5", jobsystem::WorkerMain },
        ThreadObject{ "THREAD_JOBSYSTEM_WORKER_6", jobsystem::WorkerMain },
        ThreadObject{ "THREAD_JOBSYSTEM_WORKER_7", jobsystem::WorkerMain },
        ThreadObject{ "THREAD_JOBSYSTEM_WORKER_8", jobsystem::WorkerMain },
#endif
    };
} s_threadGlob;

bool Initialize() {
    SetThreadId(THREAD_MAIN);

    std::latch latch{ THREAD_MAX - 1 };

    // skip main thread
    for (uint32_t id = THREAD_MAIN + 1; id < THREAD_MAX; ++id) {
        ThreadObject& thread = s_threadGlob.threads[id];
        thread.id = id;
        thread.threadObject = std::thread(
            [&](ThreadObject* p_object) {
                // set thread id
                SetThreadId(p_object->id);

                latch.count_down();
                LOG_VERBOSE("[threads] +{}#{}", p_object->name, p_object->id);
                CAVE_PROFILE_THREAD(p_object->name);
                p_object->threadFunc();
                LOG_VERBOSE("[threads] -{}#{}", p_object->name, p_object->id);
            },
            &thread);

        SetThreadName(thread.threadObject, thread.name);
    }

    latch.wait();
    return true;
}

void Finailize() {
    for (uint32_t id = THREAD_MAIN + 1; id < THREAD_MAX; ++id) {
        auto& thread = s_threadGlob.threads[id].threadObject;
        if (thread.joinable()) {
            thread.join();
        }
    }
}

bool ShutdownRequested() {
    return s_threadGlob.shutdownRequested;
}

void RequestShutdown() {
    s_threadGlob.shutdownRequested = true;
    // wake up
}

bool IsMainThread() {
    return g_thread_id == THREAD_MAIN;
}

uint32_t GetThreadId() {
    return g_thread_id;
}

void SetThreadName(std::thread& p_thread, std::string_view p_name) {
#if USING(PLATFORM_WINDOWS)
    HANDLE handle = (HANDLE)p_thread.native_handle();

    std::wstring name(p_name.begin(), p_name.end());
    HRESULT hr = ::SetThreadDescription(handle, name.c_str());
    DEV_ASSERT(!FAILED(hr));
#endif
}

void SetThreadId(uint32_t p_id) {
    g_thread_id = p_id;
}

}  // namespace cave::thread
