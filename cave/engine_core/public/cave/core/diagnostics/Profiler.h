// =============================================================================
// File: cave/core/diagnostics/Profiler.h
// =============================================================================
#pragma once
#include "cave/core/PlatformDefines.h"

#if USING(ENABLE_PROFILER)
#include <optick/optick.h>
#define CAVE_PROFILE_FRAME(...)  OPTICK_FRAME(__VA_ARGS__)
#define CAVE_PROFILE_EVENT(...)  OPTICK_EVENT(__VA_ARGS__)
#define CAVE_PROFILE_THREAD(...) OPTICK_THREAD(__VA_ARGS__)
#else
#define CAVE_PROFILE_FRAME(...)  (void)0
#define CAVE_PROFILE_EVENT(...)  (void)0
#define CAVE_PROFILE_THREAD(...) (void)0
#endif
