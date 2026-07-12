#include "cave/core/diagnostics/CompositeLogger.h"

#include "engine/private/core/diagnostics/log_sink/AnsiLogSink.h"
#include "engine/private/core/io/file_access_unix.h"
#include "engine/private/core/os/os.h"
#include "engine/private/drivers/windows/Win32ConsoleSink.h"
#include "engine/private/drivers/windows/Win32DebuggerSink.h"
#include "engine/private/drivers/windows/win32_prerequisites.h"

namespace cave {

void OS::Initialize() {
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_RESOURCE);
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);

    addLogger(MakeOwner<Win32Logger>());
    addLogger(MakeOwner<DebugConsoleLogger>());

#if 0
    if (EnableAnsi()) {
        addLogger(MakeOwner<AnsiLogger>());
    }
#endif

    SetLogger(&logger_);
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
