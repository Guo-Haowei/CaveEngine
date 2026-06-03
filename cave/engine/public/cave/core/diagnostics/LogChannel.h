// =============================================================================
// File: engine/public/cave/core/diagnostics/LogChannel.h
// =============================================================================
#pragma once
#include <cstdint>

// clang-format off
#define CAVE_LOG_CHANNEL_LIST \
    CAVE_LOG_CHANNEL(Default, "[Default]") \
    CAVE_LOG_CHANNEL(App,     "[App]    ") \
    CAVE_LOG_CHANNEL(Core,    "[Core]   ") \
    CAVE_LOG_CHANNEL(Dvar,    "[Dvar]   ") \
    CAVE_LOG_CHANNEL(Editor,  "[Editor] ") \
    CAVE_LOG_CHANNEL(Game,    "[Game]   ") \
    CAVE_LOG_CHANNEL(Console, "[Console]") \
    CAVE_LOG_CHANNEL(Picking, "[Picking]") \
    CAVE_LOG_CHANNEL(Asset,   "[Asset]  ") \
    CAVE_LOG_CHANNEL(Scene,   "[Scene]  ") \
    CAVE_LOG_CHANNEL(Thumb,   "[Thumb]  ") \
    CAVE_LOG_CHANNEL(Input,   "[Input]  ") \
    CAVE_LOG_CHANNEL(Intent,  "[Intent] ") \
    CAVE_LOG_CHANNEL(UI,      "[UI]     ") \
    CAVE_LOG_CHANNEL(ImGui,   "[ImGui]  ") \
    CAVE_LOG_CHANNEL(Lua,     "[Lua]    ") \
    CAVE_LOG_CHANNEL(Physics, "[Physics]") \
    CAVE_LOG_CHANNEL(Thread,  "[Thread] ") \
    CAVE_LOG_CHANNEL(View,    "[View]   ") \
    CAVE_LOG_CHANNEL(Render,  "[Render] ") \
    CAVE_LOG_CHANNEL(RHI,     "[RHI]    ") \
    CAVE_LOG_CHANNEL(Job,     "[Job]    ")
// clang-format on

namespace cave {

enum class LogChannel : uint16_t {
#define CAVE_LOG_CHANNEL(ENUM, ...) ENUM,
    CAVE_LOG_CHANNEL_LIST
#undef CAVE_LOG_CHANNEL
        Count,
};

}  // namespace cave