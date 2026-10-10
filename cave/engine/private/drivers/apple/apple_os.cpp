#include <unistd.h>

#include "engine/private/core/diagnostics/log_sink/AnsiLogSink.h"
#include "engine/private/core/diagnostics/log_sink/StdLogSink.h"
#include "engine/private/core/io/FileAccessUnix.h"
#include "engine/private/core/os/os.h"

namespace cave {

void OS::Initialize() {
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_RESOURCE);
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
    FileAccess::MakeDefault<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);

    if (IsAnsiSupported()) {
        addLogger(MakeOwner<AnsiLogger>());
    } else {
        addLogger(MakeOwner<StdLogger>());
    }

    SetLogger(&m_logger);
}

bool IsAnsiSupported() {
    if (!isatty(STDOUT_FILENO)) {
        return false;  // Output is not a terminal
    }

    const char* term_cstring = std::getenv("TERM");
    if (term_cstring == nullptr) {
        return false;  // TERM is not set
    }

    std::string term(term_cstring);

    return (term.find("xterm") != std::string::npos ||
            term.find("vt100") != std::string::npos ||
            term.find("screen") != std::string::npos ||
            term.find("ansi") != std::string::npos);
}

}  // namespace cave
