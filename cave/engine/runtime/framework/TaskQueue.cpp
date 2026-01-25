#include "TaskQueue.h"

namespace cave {

void TaskQueue::Enqueue(std::function<void()> p_func) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(std::move(p_func));
}

void TaskQueue::Drain() {
    for (;;) {
        std::function<void()> fn;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_queue.empty()) break;
            fn = std::move(m_queue.front());
            m_queue.pop();
        }
        if (fn) fn();
    }
}

}  // namespace cave
