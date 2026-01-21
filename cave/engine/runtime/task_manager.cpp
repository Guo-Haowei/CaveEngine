#include "task_manager.h"

#include "engine/runtime/async_task_interface.h"
#include "engine/runtime/task_context.h"
#include "engine/runtime/task_queue.h"

namespace cave {

class TaskContext;

static uint64_t NowMs() {
    using namespace std::chrono;
    return (uint64_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

static double Clamp01(double v) {
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}

TaskManager::TaskManager(TaskQueue& mtq)
    : m_task_queue(mtq) {}

TaskManager::~TaskManager() {
    Stop();
}

bool TaskManager::Start(uint32_t worker_count) {
    Stop();
    if (worker_count == 0) worker_count = 1;

    m_is_running.store(true);
    m_workers.reserve(worker_count);
    for (uint32_t i = 0; i < worker_count; ++i) {
        m_workers.emplace_back([this]() { WorkerLoop(); });
    }
    return true;
}

void TaskManager::Stop() {
    if (!m_is_running.exchange(false)) return;

    m_work_cv.notify_all();
    for (auto& t : m_workers) {
        if (t.joinable()) t.join();
    }
    m_workers.clear();
}

TaskId TaskManager::Submit(std::unique_ptr<IAsyncTask> task,
                               TaskSubmitOptions opt,
                               TaskCompletionCallback on_done) {
    if (!task) return kInvalidTaskId;

    TaskId id = m_next_id.fetch_add(1);

    auto st = std::make_unique<TaskState>();
    st->id = id;
    st->name = task->Name();
    st->task = std::move(task);
    st->priority = opt.priority;
    st->start_immediately = opt.start_immediately;
    st->on_done = std::move(on_done);

    st->status.store(TaskStatus::Queued);
    st->indeterminate.store(true);
    st->progress01.store(0.0);

    {
        std::lock_guard<std::mutex> lock(m_states_mutex);
        m_states[id] = std::move(st);
    }

    if (opt.start_immediately) {
        EnqueueWork(id, opt.priority);
    }
    return id;
}

TaskId TaskManager::SubmitGroup(TaskGroupSpec spec,
                                    TaskPriority priority,
                                    TaskCompletionCallback on_done) {
    TaskId id = m_next_id.fetch_add(1);

    auto st = std::make_unique<TaskState>();
    st->id = id;
    st->name = std::move(spec.name);
    st->priority = priority;
    st->on_done = std::move(on_done);

    st->is_group = true;
    st->children = std::move(spec.children);
    st->weights = std::move(spec.weights);

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

void TaskManager::ResumeTask(TaskId id) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(id);
    if (!s) return;
    if (s->is_group) return;
    if (s->status.load() != TaskStatus::Queued) return;

    s->start_immediately = true;
    EnqueueWork(id, s->priority);
}

void TaskManager::RequestCancel(TaskId id) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(id);
    if (!s) return;

    s->cancel_requested.store(true);

    if (s->is_group) {
        for (TaskId c : s->children) {
            TaskState* cs = FindStateUnlocked(c);
            if (cs) cs->cancel_requested.store(true);
        }
    }
}

TaskSnapshot TaskManager::GetSnapshot(TaskId id) const {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    const TaskState* s = FindStateUnlocked(id);

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

std::vector<TaskLogLine> TaskManager::GetRecentLogs(TaskId id, size_t max_lines) const {
    std::vector<TaskLogLine> out;

    std::lock_guard<std::mutex> lock(m_states_mutex);
    const TaskState* s = FindStateUnlocked(id);
    if (!s) return out;

    std::lock_guard<std::mutex> l(s->log_mutex);
    const size_t n = std::min(max_lines, s->logs.size());
    out.reserve(n);

    auto it = s->logs.end();
    for (size_t i = 0; i < n; ++i) --it;
    for (; it != s->logs.end(); ++it) out.push_back(*it);

    return out;
}

void TaskManager::TickMainThread() {
    // You may drain outside, but doing it here is convenient.
    m_task_queue.Drain();

    // Update groups + dispatch group completion callbacks.
    std::lock_guard<std::mutex> lock(m_states_mutex);
    for (auto& kv : m_states) {
        TaskState& s = *kv.second;
        if (!s.is_group) continue;
        UpdateGroupAggregation(s);
        MaybeEnqueueCompletionOnMainThread(s);
    }
}

void TaskManager::WorkerLoop() {
    while (m_is_running.load()) {
        TaskId id = kInvalidTaskId;
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

        TaskContext ctx(*this, m_task_queue, id);

        try {
            if (task) task->Run(ctx);
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

bool TaskManager::PopNextWorkItem(TaskId& out_id) {
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

void TaskManager::EnqueueWork(TaskId id, TaskPriority pri) {
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
    m_work_cv.notify_one();
}

TaskManager::TaskState* TaskManager::FindStateUnlocked(TaskId id) {
    auto it = m_states.find(id);
    if (it == m_states.end()) return nullptr;
    return it->second.get();
}

const TaskManager::TaskState* TaskManager::FindStateUnlocked(TaskId id) const {
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

    TaskId id = s.id;
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

    m_task_queue.Enqueue([id, snap, cb]() mutable {
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

    double total_w = 0.0;
    double sum = 0.0;

    for (size_t i = 0; i < g.children.size(); ++i) {
        TaskId cid = g.children[i];
        TaskState* c = FindStateUnlocked(cid);
        if (!c) continue;

        const double w = has_weights ? g.weights[i] : 1.0;
        total_w += w;

        TaskStatus cs = c->status.load();
        if (cs == TaskStatus::Failed) any_failed = true;
        if (cs == TaskStatus::Queued || cs == TaskStatus::Running) {
            all_done = false;
            any_running = true;
        }

        // Child contribution:
        double cp = 0.0;
        if (cs == TaskStatus::Succeeded)
            cp = 1.0;
        else if (cs == TaskStatus::Failed || cs == TaskStatus::Canceled)
            cp = 1.0;  // group progresses past completed children
        else {
            if (c->indeterminate.load()) {
                // For indeterminate child, treat as 0 until it becomes determinate.
                cp = 0.0;
            } else {
                cp = Clamp01(c->progress01.load());
            }
        }

        sum += w * cp;
    }

    if (total_w <= 0.0) total_w = 1.0;
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

void TaskManager::CtxSetIndeterminate(TaskId id, bool v) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(id);
    if (!s) return;
    s->indeterminate.store(v);
}

void TaskManager::CtxSetProgress(TaskId id, double p01) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(id);
    if (!s) return;
    s->indeterminate.store(false);
    s->progress01.store(Clamp01(p01));
}

void TaskManager::CtxFail(TaskId id, std::string err) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(id);
    if (!s) return;

    {
        std::lock_guard<std::mutex> el(s->err_mutex);
        s->last_error = std::move(err);
    }
    s->status.store(TaskStatus::Failed);
}

bool TaskManager::CtxIsCancelRequested(TaskId id) const {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    const TaskState* s = FindStateUnlocked(id);
    if (!s) return false;
    return s->cancel_requested.load();
}

void TaskManager::CtxLog(TaskId id, TaskLogLevel lvl, std::string msg) {
    std::lock_guard<std::mutex> lock(m_states_mutex);
    TaskState* s = FindStateUnlocked(id);
    if (!s) return;

    std::lock_guard<std::mutex> l(s->log_mutex);
    if (s->logs.size() >= s->log_capacity) s->logs.pop_front();
    s->logs.push_back(TaskLogLine{ NowMs(), lvl, std::move(msg) });
}

}  // namespace cave::tmp
