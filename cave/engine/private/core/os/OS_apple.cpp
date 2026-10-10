#include <unistd.h>

#include "engine/private/core/diagnostics/log_sink/StdLogSink.h"
#include "engine/private/core/os/OS.h"
#include "engine/private/io/FileAccessUnix.h"

namespace cave {

void OS::initialize() {
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_RESOURCE);
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);

    addLogger(MakeOwner<StdLogger>());

    SetLogger(&m_logger);
}

void OS::finalize() {
    SetLogger(nullptr);
}

}  // namespace cave
