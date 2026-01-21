#include "task_context.h"

#include "engine/runtime/task_manager.h"
#include "engine/runtime/task_queue.h"

namespace cave {

void TaskContext::SetIndeterminate(bool v) { m_task_manager.CtxSetIndeterminate(m_task_id, v); }
void TaskContext::SetProgress(double p01) { m_task_manager.CtxSetProgress(m_task_id, p01); }
void TaskContext::RequestCancel() { m_task_manager.RequestCancel(m_task_id); }
bool TaskContext::IsCancelRequested() const { return m_task_manager.CtxIsCancelRequested(m_task_id); }
void TaskContext::Fail(std::string err) { m_task_manager.CtxFail(m_task_id, std::move(err)); }
void TaskContext::Log(TaskLogLevel lvl, std::string msg) { m_task_manager.CtxLog(m_task_id, lvl, std::move(msg)); }
void TaskContext::EnqueueMainThread(std::function<void()> fn) { m_task_queue.Enqueue(std::move(fn)); }

}  // namespace cave
