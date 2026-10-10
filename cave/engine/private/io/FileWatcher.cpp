#include "cave/io/FileWatcher.h"

// @TODO: refactor
#if USING(PLATFORM_WINDOWS)
#include "engine/private/drivers/windows/win32_prerequisites.h"
#elif USING(PLATFORM_APPLE)
#else
#error "Platform not supported"
#endif

namespace cave {

FileWatcher::FileWatcher()
    : m_dir_handle{ INVALID_HANDLE_VALUE } {
}

void FileWatcher::start(std::string_view path) {
    m_path = path;
    m_stop = false;

    m_thread = std::thread([this]() {
        watchLoop();
    });
}

void FileWatcher::stop() {
    m_stop = true;
#if USING(PLATFORM_WINDOWS)
    if (m_dir_handle != INVALID_HANDLE_VALUE) {
        ::CancelIoEx(m_dir_handle, nullptr);
    }

    if (m_thread.joinable()) {
        m_thread.join();
    }

    if (m_dir_handle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(m_dir_handle);
        m_dir_handle = INVALID_HANDLE_VALUE;
    }
#endif
}

void FileWatcher::watchLoop() {
#if USING(PLATFORM_WINDOWS)
    std::wstring path(m_path.begin(), m_path.end());

    m_dir_handle = ::CreateFileW(
        path.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (m_dir_handle == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Failed to open directory {}", m_path);
        return;
    }

    constexpr DWORD bufferSize = 8192;
    BYTE buffer[bufferSize];
    DWORD bytesReturned;

    while (!m_stop.load()) {
        BOOL success = ReadDirectoryChangesW(
            m_dir_handle,
            buffer,
            bufferSize,
            TRUE,  // recursive
            FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            nullptr,
            nullptr);

        if (!success || m_stop.load()) {
            break;
        }

        m_changed.store(true);  // flag to main thread
    }
#endif
}

}  // namespace cave
