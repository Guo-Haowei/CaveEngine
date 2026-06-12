#include "cave/core/ErrorMacros.h"

#include "engine/private/core/os/os.h"
#include "engine/private/drivers/windows/win32_prerequisites.h"

namespace cave {

IntrusiveList<ErrorHandlerListNode> s_errorHandlers;

void GlobalLock() {}
void GlobalUnlock() {}

void BreakIfDebug() {
#if USING(PLATFORM_WINDOWS)
    if (IsDebuggerPresent()) {
        GENERATE_TRAP();
    }
#endif
}

bool AddErrorHandler(ErrorHandler* handler) {
    // if the handler already exists, remove it
    RemoveErrorHandler(handler);

    GlobalLock();
    s_errorHandlers.node_push_front(handler);
    GlobalUnlock();
    return true;
}

bool RemoveErrorHandler(const ErrorHandler* handler) {
    GlobalLock();
    s_errorHandlers.node_remove(handler);
    GlobalUnlock();
    return true;
}

void ReportErrorImpl(std::string_view function,
                     std::string_view file,
                     int line,
                     std::string_view error,
                     std::string_view detail) {
    std::string extra;
    if (!detail.empty()) {
        extra = std::format("\nDetail: {}", detail);
    }

    auto message = std::format("ERROR: {}{}\n    at {} ({}:{})\n",
                               error,
                               extra,
                               function,
                               file,
                               line);
    LogImpl(LOG_LEVEL_ERROR, std::move(message));

    GlobalLock();
    for (auto& handler : s_errorHandlers) {
        handler.errorFunc(handler.userdata, function, file, line, error);
    }
    GlobalUnlock();
}

void ReportErrorIndexImpl(std::string_view p_function,
                          std::string_view p_file,
                          int p_line,
                          std::string_view p_prefix,
                          int64_t p_index,
                          int64_t p_bound,
                          std::string_view p_index_string,
                          std::string_view p_bound_string,
                          std::string_view p_detail) {
    auto error2 = std::format("{}Index {} = {} is out of bounds ({} = {}).", p_prefix, p_index_string, p_index, p_bound_string, p_bound);

    ReportErrorImpl(p_function, p_file, p_line, error2, p_detail);
}

}  // namespace cave
