#pragma once

namespace cave {

class TaskQueue {
public:
    void Enqueue(std::function<void()> p_func);

    void Drain();

private:
    std::mutex m_mutex;
    std::queue<std::function<void()>> m_queue;
};

}  // namespace cave
