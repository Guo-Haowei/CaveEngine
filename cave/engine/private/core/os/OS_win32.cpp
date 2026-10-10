#include "cave/core/diagnostics/CompositeLogger.h"

#include "engine/private/core/diagnostics/log_sink/AnsiLogSink.h"
#include "engine/private/io/FileAccessUnix.h"
#include "engine/private/core/os/os.h"
#include "engine/private/drivers/windows/Win32ConsoleSink.h"
#include "engine/private/drivers/windows/Win32DebuggerSink.h"
#include "engine/private/drivers/windows/win32_prerequisites.h"

namespace cave {

void OS::initialize() {
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_RESOURCE);
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);

    addLogger(MakeOwner<Win32Logger>());
    addLogger(MakeOwner<DebugConsoleLogger>());

    SetLogger(&m_logger);
}

void OS::finalize() {
    SetLogger(nullptr);
}

}  // namespace cave
