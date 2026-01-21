#pragma once
#include "engine/runtime/module.h"
#include "engine/runtime/task_context.h"

namespace cave {

class IAsyncTask;
class TaskQueue;

struct TaskGroupSpec {
    std::string name;
    std::vector<uint64_t> children;
    std::vector<float> weights;  // optional; if empty => equal weights
};

struct TaskSnapshot {
    uint64_t id = kInvalidTaskId;
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
    using TaskCompletionCallback = std::function<void(uint64_t, TaskSnapshot)>;

public:
    explicit TaskManager();
    ~TaskManager();

    void Stop();

    // Long task submission
    uint64_t Submit(std::unique_ptr<IAsyncTask> p_task,
                  TaskSubmitOptions p_opt = {},
                  TaskCompletionCallback p_on_done = nullptr);

    // Aggregates children progress and completes when all children complete.
    uint64_t SubmitGroup(TaskGroupSpec p_spec,
                       TaskPriority p_priority = TaskPriority::Normal,
                       TaskCompletionCallback p_on_done = nullptr);

    // Cancel cooperatively.
    void RequestCancel(uint64_t p_id);

    // If you submit with start_immediately=false, call this to queue it.
    void ResumeTask(uint64_t p_id);

    // Read-only views for UI
    TaskSnapshot GetSnapshot(uint64_t p_id) const;

    bool HasPendingWork() const;

    void WaitUntilIdle();

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
        uint64_t id = kInvalidTaskId;
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
        std::vector<uint64_t> children;
        std::vector<float> weights;
        bool completion_enqueued = false;
    };

    struct QueuedItem {
        uint64_t id = kInvalidTaskId;
        TaskPriority pri = TaskPriority::Normal;
        uint64_t seq = 0;  // FIFO within same priority
    };

    // Worker logic
    void WorkerLoop();
    bool PopNextWorkItem(uint64_t& out_id);

    // Helpers
    TaskState* FindStateUnlocked(uint64_t id);
    const TaskState* FindStateUnlocked(uint64_t id) const;

    void EnqueueWork(uint64_t id, TaskPriority pri);
    void MaybeEnqueueCompletionOnMainThread(TaskState& s);

    // Called by TaskContext
    void ContxtSetIndeterminate(uint64_t id, bool v);
    void ContextSetProgress(uint64_t id, float p01);
    void ContextFail(uint64_t id, std::string err);
    bool ContextIsCancelRequested(uint64_t id) const;
    void CtxLog(uint64_t id, TaskLogLevel lvl, std::string msg);

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
    std::unordered_map<uint64_t, std::unique_ptr<TaskState>> m_states;
    std::atomic<uint64_t> m_next_id{ 1 };
    std::atomic<int> m_in_flight{ 0 };
};

}  // namespace cave
