#include "cave/core/threading/Threads.h"
#include "cave/core/threading/JobSystem.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/core/diagnostics/Profiler.h"

#if USING(PLATFORM_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <array>
#include <latch>
#include <thread>

namespace cave::thread {

using ThreadMainFunc = void (*)();

struct ThreadObject {
    const char* name;
    ThreadMainFunc threadFunc;
    uint32_t id{ 0 };
    std::thread threadObject{};
};

static thread_local uint32_t s_thread_id;

static struct {
    std::atomic_bool shutdownRequested;
    std::array<ThreadObject, THREAD_MAX> threads = {
        ThreadObject{ "THREAD_MAIN", nullptr },
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
            [&](ThreadObject* object) {
                // set thread id
                SetThreadId(object->id);

                latch.count_down();
                LOG_TRACE(LogChannel::Thread, "+{}#{}", object->name, object->id);
                CAVE_PROFILE_THREAD(object->name);
                object->threadFunc();
                LOG_TRACE(LogChannel::Thread, "-{}#{}", object->name, object->id);
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
    return s_thread_id == THREAD_MAIN;
}

uint32_t GetThreadId() {
    return s_thread_id;
}

bool SetThreadName(std::thread& thread, std::string_view name) {
#if USING(PLATFORM_WINDOWS)
    HANDLE handle = (HANDLE)thread.native_handle();

    std::wstring wname(name.begin(), name.end());
    HRESULT hr = ::SetThreadDescription(handle, wname.c_str());
    return !FAILED(hr);
#endif
    return true;
}

void SetThreadId(uint32_t id) {
    s_thread_id = id;
}

}  // namespace cave::thread
