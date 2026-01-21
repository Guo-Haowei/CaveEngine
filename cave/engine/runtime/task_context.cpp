#include "task_context.h"

#include "engine/runtime/task_manager.h"
#include "engine/runtime/task_queue.h"

namespace cave {

void TaskContext::SetIndeterminate(bool p_value) {
    m_task_manager.CtxSetIndeterminate(m_task_id, p_value);
}

void TaskContext::SetProgress(double p_progress) {
    m_task_manager.CtxSetProgress(m_task_id, p_progress);
}

void TaskContext::RequestCancel() {
    m_task_manager.RequestCancel(m_task_id);
}

bool TaskContext::IsCancelRequested() const {
    return m_task_manager.CtxIsCancelRequested(m_task_id);
}

void TaskContext::Fail(std::string p_error) {
    m_task_manager.CtxFail(m_task_id, std::move(p_error));
}

void TaskContext::Log(TaskLogLevel p_level, std::string p_message) {
    m_task_manager.CtxLog(m_task_id, p_level, std::move(p_message));
}

}  // namespace cave
