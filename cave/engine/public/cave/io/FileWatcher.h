#pragma once
#include <atomic>
#include <string>
#include <thread>

namespace cave {

class FileWatcher {
public:
    explicit FileWatcher();

    void start(std::string_view path);
    void stop();

    bool hasChanged() const { return m_changed.load(); }

    void clearFlag() { m_changed.store(false); }

    bool isStopped() const { return m_stop; }

private:
    void watchLoop();

    std::string m_path;
    std::thread m_thread;
    std::atomic<bool> m_stop{ true };
    std::atomic<bool> m_changed{ true };  // set to true to trigger build the first frame

    void* m_dir_handle{ nullptr };
};

}  // namespace cave
