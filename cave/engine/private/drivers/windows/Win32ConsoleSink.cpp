#include "Win32ConsoleSink.h"

#include "engine/private/core/diagnostics/log_sink/LogUtils.h"
#include "engine/private/drivers/windows/win32_prerequisites.h"

namespace cave {

static WORD FindColorAttribute(LogLevel level) {
    switch (level) {
#define LOG_LEVEL_COLOR(LEVEL, TAG, ANSI, WINCOLOR) \
    case LEVEL:                                     \
        return WINCOLOR;
        LOG_LEVEL_COLOR_LIST
#undef LOG_LEVEL_COLOR
        default:
            return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
}

void Win32Logger::Submit(const LogEvent& log) {
    const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO buffer_info;
    const WORD new_color = FindColorAttribute(log.level);

    // @TODO: stderr vs stdout
    FILE* file = stdout;
    fflush(file);

    m_console_mutex.lock();
    GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
    const WORD old_color_attrs = buffer_info.wAttributes;
    SetConsoleTextAttribute(stdout_handle, new_color);
    fprintf(file, "%s  %s  %s  %s\n",
            log.time_str,
            ToString(log.level),
            ToString(log.channel),
            log.message.c_str());
    SetConsoleTextAttribute(stdout_handle, old_color_attrs);
    fflush(file);
    m_console_mutex.unlock();
}

}  // namespace cave
