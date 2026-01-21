#pragma once

namespace cave {

class TaskManager;

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

inline constexpr uint64_t kInvalidTaskId = 0;

class TaskContext {
public:
    TaskContext(TaskManager& p_manager, uint64_t p_task_id)
        : m_task_manager(p_manager), m_task_id(p_task_id) {}

    uint64_t Id() const { return m_task_id; }

    // Progress
    void SetIndeterminate(bool p_value);
    void SetProgress(float p_progress);  // clamps [0,1], sets indeterminate=false

    // Cancel/fail
    void RequestCancel();
    bool IsCancelRequested() const;
    void Fail(std::string p_error);  // marks Failed; task should return

    // Main thread continuation (GPU upload, registry mutation, etc.)
    // void EnqueueMainThread(std::function<void()> p_func);

private:
    TaskManager& m_task_manager;
    uint64_t m_task_id;
};

}  // namespace cave
