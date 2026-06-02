#include "engine/private/core/diagnostics/logger/AnsiLogger.h"
#include "engine/private/core/io/file_access_unix.h"
#include "engine/private/core/os/os.h"
#include "win32_logger.h"
#include "win32_prerequisites.h"

namespace cave {

class DebugConsoleLogger : public ILogger {
public:
    void Print(const LogEvent& p_log) override;
};

void DebugConsoleLogger::Print(const LogEvent& p_log) {
    OutputDebugStringA(p_log.message.c_str());
}

void OS::Initialize() {
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_RESOURCE);
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);

    AddLogger(std::make_shared<Win32Logger>());
    // if (EnableAnsi()) {
    //     AddLogger(std::make_shared<AnsiLogger>());
    // }

    AddLogger(std::make_shared<DebugConsoleLogger>());
}

bool IsAnsiSupported() {
    HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (console == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD mode = 0;
    if (!::GetConsoleMode(console, &mode)) {
        return false;
    }

    return mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING;
}

bool EnableAnsi() {
    HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (console == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD mode = 0;
    if (!::GetConsoleMode(console, &mode)) {
        return false;
    }

    if (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) {
        return true;
    }

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!::SetConsoleMode(console, mode)) {
        return false;
    }

    return true;
}

}  // namespace cave
