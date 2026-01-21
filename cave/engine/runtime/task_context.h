#pragma once

namespace cave {

class TaskManager;
class TaskQueue;

enum class TaskLogLevel {
    Info,
    Warn,
    Error
};

enum class TaskStatus {
    Queued,
    Running,
    Succeeded,
    Failed,
    Canceled
};

using TaskId = uint64_t;
inline constexpr TaskId kInvalidTaskId = 0;

class TaskContext {
public:
    TaskContext(TaskManager& p_manager, TaskQueue& p_thread_queue, TaskId p_task_id)
        : m_task_manager(p_manager), m_task_queue(p_thread_queue), m_task_id(p_task_id) {}

    TaskId Id() const { return m_task_id; }

    // Progress
    void SetIndeterminate(bool p_value);
    void SetProgress(double p_progress);  // clamps [0,1], sets indeterminate=false

    // Cancel/fail
    void RequestCancel();
    bool IsCancelRequested() const;
    void Fail(std::string p_error);  // marks Failed; task should return

    // Logs
    void Log(TaskLogLevel p_log_level, std::string p_message);

    // Main thread continuation (GPU upload, registry mutation, etc.)
    void EnqueueMainThread(std::function<void()> p_func);

private:
    TaskManager& m_task_manager;
    TaskQueue& m_task_queue;
    TaskId m_task_id;
};

}  // namespace cave

