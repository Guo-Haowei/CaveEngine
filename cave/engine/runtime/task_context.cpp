#include "task_context.h"

#include "engine/runtime/task_manager.h"
#include "engine/runtime/task_queue.h"

namespace cave {

void TaskContext::SetIndeterminate(bool p_value) {
    m_task_manager.ContxtSetIndeterminate(m_task_id, p_value);
}

void TaskContext::SetProgress(float p_progress) {
    m_task_manager.ContextSetProgress(m_task_id, p_progress);
}

void TaskContext::RequestCancel() {
    m_task_manager.RequestCancel(m_task_id);
}

bool TaskContext::IsCancelRequested() const {
    return m_task_manager.ContextIsCancelRequested(m_task_id);
}

void TaskContext::Fail(std::string p_error) {
    LOG_ERROR("{}", p_error);
    m_task_manager.ContextFail(m_task_id, std::move(p_error));
}

}  // namespace cave
