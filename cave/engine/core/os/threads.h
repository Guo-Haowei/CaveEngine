#pragma once
#include "engine/systems/job_system/job_system.h"

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

void SetThreadName(std::thread& p_thread, std::string_view p_name);

void SetThreadId(uint32_t p_id);

}  // namespace cave::thread
