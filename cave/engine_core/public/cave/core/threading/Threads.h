#pragma once
#include <string_view>
#include <thread>
#include "cave/core/PlatformDefines.h"

namespace cave::thread {

// @TODO: refactor thread
enum : uint32_t {
    THREAD_MAIN,
#if USING(ENABLE_JOB_SYSTEM)
    THREAD_JOBSYSTEM_WORKER_1,
    THREAD_JOBSYSTEM_WORKER_2,
    THREAD_JOBSYSTEM_WORKER_3,
    THREAD_JOBSYSTEM_WORKER_4,
    THREAD_JOBSYSTEM_WORKER_5,
    THREAD_JOBSYSTEM_WORKER_6,
    THREAD_JOBSYSTEM_WORKER_7,
    THREAD_JOBSYSTEM_WORKER_8,
#endif
    THREAD_MAX,

    THREAD_TASK_MANAGER_WORKER_1,
    THREAD_TASK_MANAGER_WORKER_2,
};

bool Initialize();

void Finailize();

bool ShutdownRequested();

void RequestShutdown();

bool IsMainThread();

uint32_t GetThreadId();

bool SetThreadName(std::thread& thread, std::string_view name);

void SetThreadId(uint32_t id);

}  // namespace cave::thread
