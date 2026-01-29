#include "TaskManager.h"

#include "engine/private/core/os/threads.h"
#include "engine/private/runtime/framework/IAsyncTask.h"
#include "engine/private/runtime/framework/TaskContext.h"
#include "engine/private/runtime/framework/TaskQueue.h"

namespace cave {

class TaskContext;

static float Clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

TaskManager::TaskManager()
    : Module("TaskManager")
    , m_task_queue(std::make_unique<TaskQueue>()) {
}

TaskManager::~TaskManager() {
    Stop();
}

auto TaskManager::InitializeImpl() -> Result<void> {
    Stop();

    uint32_t worker_count = 1;

    m_is_running.store(true);
    m_workers.reserve(worker_count);
    for (uint32_t i = 0; i < worker_count; ++i) {
        std::thread thread([this](uint32_t p_id) { WorkerLoop(p_id); }, i);
        std::string name = std::format("THREAD_TASK_MANAGER_WORKER_{}", i);
        thread::SetThreadName(thread, name);

        m_workers.emplace_back(std::move(thread));
    }

    return Result<void>();
}

void TaskManager::FinalizeImpl() {
    Stop();
}

void TaskManager::Stop() {
    if (!m_is_running.exchange(false)) return;

    m_work_cv.notify_all();
    for (auto& t : m_workers) {
        if (t.joinable()) t.join();
    }
    m_workers.clear();
}

uint64_t TaskManager::Submit(std::unique_ptr<IAsyncTask> p_task,
                             TaskSubmitOptions p_opt,
                             TaskCompletionCallback p_on_done) {
    if (!p_task) {
        return kInvalidTaskId;
    }

    uint64_t id = m_next_id.fetch_add(1);

    auto st = std::make_unique<TaskState>();
    st->id = id;
    st->name = p_task->Name();
    st->task = std::move(p_task);
    st->priority = p_opt.priority;
    st->start_immediately = p_opt.start_immediately;
    st->on_done = std::move(p_on_done);

    st->status.store(TaskStatus::Queued);
    st->indeterminate.store(true);
    st->progress01.store(0.0);

    {
        std::lock_guard<std::mutex> lock(m_states_mutex);
        m_states[id] = std::move(st);
    }

    if (p_opt.start_immediately) {
        EnqueueWork(id, p_opt.priority);
    }
    return id;
}

uint64_t TaskManager::SubmitGroup(TaskGroupSpec p_spec,
                                  TaskPriority p_priority,
                                  TaskCompletionCallback p_on_done) {
    uint64_t id = m_next_id.fetch_add(1);

    auto st = std::make_unique<TaskState>();
    st->id = id;
    st->name = std::move(p_spec.name);
    st->priority = p_priority;
    st->on_done = std::move(p_on_done);

    st->is_group = true;
    st->children = std::move(p_spec.children);
    st->weights = std::move(p_spec.weights);

    st->status.store(TaskStatus::Running);
    st->indeterminate.store(false);
    st->progress01.store(0.0);

    {
        std::lock_guard<std::mutex> lock(m_states_mutex);
        m_states[id] = std::move(st);
    }

    // Group completion is driven by TickMainThread() aggregation.
    return id;
}

void TaskManager::ResumeTask(uint64_t p_id) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(p_id);
    if (!s) return;
    if (s->is_group) return;
    if (s->status.load() != TaskStatus::Queued) return;

    s->start_immediately = true;
    EnqueueWork(p_id, s->priority);
}

void TaskManager::RequestCancel(uint64_t p_id) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(p_id);
    if (!s) return;

    s->cancel_requested.store(true);

    if (s->is_group) {
        for (uint64_t c : s->children) {
            TaskState* cs = FindStateUnlocked(c);
            if (cs) cs->cancel_requested.store(true);
        }
    }
}

TaskSnapshot TaskManager::GetSnapshot(uint64_t p_id) const {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    const TaskState* s = FindStateUnlocked(p_id);

    TaskSnapshot out;
    if (!s) return out;

    out.id = s->id;
    out.name = s->name;
    out.status = s->status.load();
    out.indeterminate = s->indeterminate.load();
    out.progress01 = s->progress01.load();
    out.cancel_requested = s->cancel_requested.load();
    {
        std::lock_guard<std::mutex> el(s->err_mutex);
        out.last_error = s->last_error;
    }
    return out;
}

void TaskManager::TickMainThread() {
    // You may drain outside, but doing it here is convenient.
    m_task_queue->Drain();

    // Update groups + dispatch group completion callbacks.
    std::lock_guard<std::mutex> lock(m_states_mutex);
    for (auto& kv : m_states) {
        TaskState& s = *kv.second;
        if (!s.is_group) continue;
        UpdateGroupAggregation(s);
        MaybeEnqueueCompletionOnMainThread(s);
    }
}

bool TaskManager::HasPendingWork() const {
    return m_in_flight.load(std::memory_order_acquire) > 0;
}

void TaskManager::WaitUntilIdle() {
    while (HasPendingWork()) {
        std::this_thread::yield();
    }
}

void TaskManager::WorkerLoop(uint32_t p_worker_id) {
    thread::SetThreadId(thread::THREAD_TASK_MANAGER_WORKER_1 + p_worker_id);

    while (m_is_running.load()) {
        uint64_t id = kInvalidTaskId;
        if (!PopNextWorkItem(id)) continue;

        std::unique_ptr<IAsyncTask> task;
        {
            std::lock_guard<std::mutex> lock(m_states_mutex);
            TaskState* s = FindStateUnlocked(id);
            if (!s) continue;
            if (s->is_group) continue;
            if (s->status.load() != TaskStatus::Queued) continue;

            // mark running
            s->status.store(TaskStatus::Running);

            // already canceled?
            if (s->cancel_requested.load()) {
                s->status.store(TaskStatus::Canceled);
                MaybeEnqueueCompletionOnMainThread(*s);
                continue;
            }

            // Move task out so no concurrent touches during Run().
            task = std::move(s->task);
        }

        TaskContext ctx(*this, id);

        try {
            if (task) {
                task->Run(ctx);
                m_in_flight.fetch_sub(1, std::memory_order_acq_rel);
            }
        } catch (...) {
            ctx.Fail("Unhandled exception in async task.");
        }

        {
            std::lock_guard<std::mutex> lock(m_states_mutex);
            TaskState* s = FindStateUnlocked(id);
            if (!s) continue;

            // move task back (optional, useful for debugging/introspection)
            s->task = std::move(task);

            // If already failed/canceled, keep it.
            TaskStatus cur = s->status.load();
            if (cur != TaskStatus::Failed && cur != TaskStatus::Canceled) {
                if (s->cancel_requested.load()) {
                    s->status.store(TaskStatus::Canceled);
                } else {
                    s->status.store(TaskStatus::Succeeded);
                    if (!s->indeterminate.load()) s->progress01.store(1.0);
                }
            }

            MaybeEnqueueCompletionOnMainThread(*s);
        }
    }
}

bool TaskManager::PopNextWorkItem(uint64_t& out_id) {
    std::unique_lock<std::mutex> lock(m_work_mutex);
    m_work_cv.wait(lock, [&]() {
        return !m_is_running.load() || !m_queue_high.empty() || !m_queue_norm.empty() || !m_queue_low.empty();
    });

    if (!m_is_running.load()) return false;

    auto pop = [&](std::deque<QueuedItem>& q) -> bool {
        if (q.empty()) return false;
        out_id = q.front().id;
        q.pop_front();
        return true;
    };

    if (pop(m_queue_high)) return true;
    if (pop(m_queue_norm)) return true;
    if (pop(m_queue_low)) return true;

    return false;
}

void TaskManager::EnqueueWork(uint64_t id, TaskPriority pri) {
    {
        std::lock_guard<std::mutex> lock(m_work_mutex);
        QueuedItem it;
        it.id = id;
        it.pri = pri;
        it.seq = m_enqueue_seq++;

        switch (pri) {
            case TaskPriority::High:
                m_queue_high.push_back(it);
                break;
            case TaskPriority::Normal:
                m_queue_norm.push_back(it);
                break;
            case TaskPriority::Low:
                m_queue_low.push_back(it);
                break;
        }
    }

    m_in_flight.fetch_add(1, std::memory_order_relaxed);
    m_work_cv.notify_one();
}

TaskManager::TaskState* TaskManager::FindStateUnlocked(uint64_t id) {
    auto it = m_states.find(id);
    if (it == m_states.end()) return nullptr;
    return it->second.get();
}

const TaskManager::TaskState* TaskManager::FindStateUnlocked(uint64_t id) const {
    auto it = m_states.find(id);
    if (it == m_states.end()) return nullptr;
    return it->second.get();
}

void TaskManager::MaybeEnqueueCompletionOnMainThread(TaskState& s) {
    if (!s.on_done) return;

    TaskStatus st = s.status.load();
    if (st != TaskStatus::Succeeded && st != TaskStatus::Failed && st != TaskStatus::Canceled) return;
    if (s.completion_enqueued) return;

    s.completion_enqueued = true;

    uint64_t id = s.id;
    TaskCompletionCallback cb = s.on_done;

    // Capture a snapshot by value for the callback.
    TaskSnapshot snap;
    snap.id = s.id;
    snap.name = s.name;
    snap.status = st;
    snap.indeterminate = s.indeterminate.load();
    snap.progress01 = s.progress01.load();
    snap.cancel_requested = s.cancel_requested.load();
    {
        std::lock_guard<std::mutex> el(s.err_mutex);
        snap.last_error = s.last_error;
    }

    m_task_queue->Enqueue([id, snap, cb]() mutable {
        cb(id, std::move(snap));
    });
}

void TaskManager::UpdateGroupAggregation(TaskState& g) {
    if (!g.is_group) return;

    if (g.children.empty()) {
        g.progress01.store(1.0);
        g.status.store(TaskStatus::Succeeded);
        return;
    }

    const bool has_weights = (!g.weights.empty() && g.weights.size() == g.children.size());

    bool any_failed = false;
    bool all_done = true;
    bool any_running = false;

    float total_w = 0.0f;
    float sum = 0.0f;

    for (size_t i = 0; i < g.children.size(); ++i) {
        uint64_t cid = g.children[i];
        TaskState* c = FindStateUnlocked(cid);
        if (!c) continue;

        const float w = has_weights ? g.weights[i] : 1.0f;
        total_w += w;

        TaskStatus cs = c->status.load();
        if (cs == TaskStatus::Failed) any_failed = true;
        if (cs == TaskStatus::Queued || cs == TaskStatus::Running) {
            all_done = false;
            any_running = true;
        }

        // Child contribution:
        float cp = 0.0f;
        if (cs == TaskStatus::Succeeded)
            cp = 1.0f;
        else if (cs == TaskStatus::Failed || cs == TaskStatus::Canceled)
            cp = 1.0f;  // group progresses past completed children
        else {
            if (c->indeterminate.load()) {
                // For indeterminate child, treat as 0 until it becomes determinate.
                cp = 0.0f;
            } else {
                cp = Clamp01(c->progress01.load());
            }
        }

        sum += w * cp;
    }

    if (total_w <= 0.0) total_w = 1.0f;
    g.indeterminate.store(false);
    g.progress01.store(Clamp01(sum / total_w));

    if (g.cancel_requested.load()) {
        // If user cancels the group, mark canceled once children are done/canceled.
        if (all_done)
            g.status.store(TaskStatus::Canceled);
        else
            g.status.store(TaskStatus::Running);
        return;
    }

    if (any_failed && all_done) {
        g.status.store(TaskStatus::Failed);
        {
            std::lock_guard<std::mutex> el(g.err_mutex);
            if (g.last_error.empty()) g.last_error = "One or more subtasks failed.";
        }
    } else if (all_done) {
        g.status.store(TaskStatus::Succeeded);
        g.progress01.store(1.0);
    } else if (any_running) {
        g.status.store(TaskStatus::Running);
    }
}

// --- TaskContext bridge ---

void TaskManager::ContxtSetIndeterminate(uint64_t id, bool v) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(id);
    if (!s) return;
    s->indeterminate.store(v);
}

void TaskManager::ContextSetProgress(uint64_t id, float p01) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(id);
    if (!s) return;
    s->indeterminate.store(false);
    s->progress01.store(Clamp01(p01));
}

void TaskManager::ContextFail(uint64_t id, std::string err) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(id);
    if (!s) return;

    {
        std::lock_guard<std::mutex> el(s->err_mutex);
        s->last_error = std::move(err);
    }
    s->status.store(TaskStatus::Failed);
}

bool TaskManager::ContextIsCancelRequested(uint64_t id) const {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    const TaskState* s = FindStateUnlocked(id);
    if (!s) return false;
    return s->cancel_requested.load();
}

}  // namespace cave
