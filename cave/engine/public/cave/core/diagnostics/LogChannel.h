// =============================================================================
// File: engine/public/cave/core/diagnostics/LogChannel.h
// =============================================================================
#pragma once
#include <cstdint>

#define CAVE_LOG_CHANNEL_LIST \
    CAVE_LOG_CHANNEL(Default) \
    CAVE_LOG_CHANNEL(App)     \
    CAVE_LOG_CHANNEL(Core)    \
    CAVE_LOG_CHANNEL(Editor)  \
    CAVE_LOG_CHANNEL(Game)    \
    CAVE_LOG_CHANNEL(Asset)   \
    CAVE_LOG_CHANNEL(UI)      \
    CAVE_LOG_CHANNEL(Script)  \
    CAVE_LOG_CHANNEL(Physics) \
    CAVE_LOG_CHANNEL(Thread)  \
    CAVE_LOG_CHANNEL(RHI)     \
    CAVE_LOG_CHANNEL(Job)

namespace cave {

enum class LogChannel : uint16_t {
#define CAVE_LOG_CHANNEL(X) X,
    CAVE_LOG_CHANNEL_LIST
#undef CAVE_LOG_CHANNEL
        Count,
};

}  // namespace cave