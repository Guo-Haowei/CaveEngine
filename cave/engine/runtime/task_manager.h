#pragma once
#include "engine/runtime/module.h"
#include "engine/runtime/task_context.h"

namespace cave {

class IAsyncTask;
class TaskQueue;

struct TaskGroupSpec {
    std::string name;
    std::vector<TaskId> children;
    std::vector<float> weights;  // optional; if empty => equal weights
};

struct TaskSnapshot {
    TaskId id = kInvalidTaskId;
    std::string name;

    TaskStatus status = TaskStatus::Queued;

    bool indeterminate = true;
    float progress01 = 0.0f;  // valid if !indeterminate

    bool cancel_requested = false;
    std::string last_error;
};

// Task priority only applies within this async system (separate from frame jobs).
enum class TaskPriority : uint8_t {
    Low = 0,
    Normal = 1,
    High = 2
};

struct TaskSubmitOptions {
    TaskPriority priority = TaskPriority::Normal;
    bool start_immediately = true;  // if false, task stays queued until ResumeTask()
};

class TaskManager : public Module {

    // Completion callback: always invoked on main thread via MainThreadQueue.
    using TaskCompletionCallback = std::function<void(TaskId, TaskSnapshot)>;

public:
    explicit TaskManager();
    ~TaskManager();

    void Stop();

    // Long task submission
    TaskId Submit(std::unique_ptr<IAsyncTask> p_task,
                  TaskSubmitOptions p_opt = {},
                  TaskCompletionCallback p_on_done = nullptr);

    // Aggregates children progress and completes when all children complete.
    TaskId SubmitGroup(TaskGroupSpec p_spec,
                       TaskPriority p_priority = TaskPriority::Normal,
                       TaskCompletionCallback p_on_done = nullptr);

    // Cancel cooperatively.
    void RequestCancel(TaskId p_id);

    // If you submit with start_immediately=false, call this to queue it.
    void ResumeTask(TaskId p_id);

    // Read-only views for UI
    TaskSnapshot GetSnapshot(TaskId p_id) const;

    // Call once per frame on main thread:
    // - drain main-thread queue (you can also do it outside)
    // - update group aggregation and fire group completion callbacks
    void TickMainThread();

protected:
    auto InitializeImpl() -> Result<void> final;
    void FinalizeImpl() final;

private:
    friend class TaskContext;

    struct TaskState {
        TaskId id = kInvalidTaskId;
        std::string name;

        std::atomic<TaskStatus> status{ TaskStatus::Queued };
        std::atomic<bool> cancel_requested{ false };

        std::atomic<bool> indeterminate{ true };
        std::atomic<float> progress01{ 0.0 };

        TaskPriority priority = TaskPriority::Normal;
        bool start_immediately = true;

        // Completion callback runs on main thread
        TaskCompletionCallback on_done;

        // Error string
        mutable std::mutex err_mutex;
        std::string last_error;

        // Execution payload (null for group tasks)
        std::unique_ptr<IAsyncTask> task;

        // Group fields
        bool is_group = false;
        std::vector<TaskId> children;
        std::vector<float> weights;
        bool completion_enqueued = false;
    };

    struct QueuedItem {
        TaskId id = kInvalidTaskId;
        TaskPriority pri = TaskPriority::Normal;
        uint64_t seq = 0;  // FIFO within same priority
    };

    // Worker logic
    void WorkerLoop();
    bool PopNextWorkItem(TaskId& out_id);

    // Helpers
    TaskState* FindStateUnlocked(TaskId id);
    const TaskState* FindStateUnlocked(TaskId id) const;

    void EnqueueWork(TaskId id, TaskPriority pri);
    void MaybeEnqueueCompletionOnMainThread(TaskState& s);

    // Called by TaskContext
    void ContxtSetIndeterminate(TaskId id, bool v);
    void ContextSetProgress(TaskId id, float p01);
    void ContextFail(TaskId id, std::string err);
    bool ContextIsCancelRequested(TaskId id) const;
    void CtxLog(TaskId id, TaskLogLevel lvl, std::string msg);

    // Group aggregation on main thread
    void UpdateGroupAggregation(TaskState& g);

private:
    std::unique_ptr<TaskQueue> m_task_queue;

    std::atomic<bool> m_is_running{ false };
    std::vector<std::thread> m_workers;

    // Priority queues: High > Normal > Low
    mutable std::mutex m_work_mutex;
    std::condition_variable m_work_cv;
    std::deque<QueuedItem> m_queue_high;
    std::deque<QueuedItem> m_queue_norm;
    std::deque<QueuedItem> m_queue_low;
    uint64_t m_enqueue_seq = 1;

    // Task registry
    mutable std::mutex m_states_mutex;
    std::unordered_map<TaskId, std::unique_ptr<TaskState>> m_states;
    std::atomic<TaskId> m_next_id{ 1 };
};

}  // namespace cave
