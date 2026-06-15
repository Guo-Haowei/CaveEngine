// =============================================================================
// File: cave/core/PlatformDefines.h
// =============================================================================
#pragma once
#include "cave/core/typedefs.h"

#define ENABLE_JOB_SYSTEM USE_IF(!USING(PLATFORM_WASM))
#define ENABLE_PROFILER   USE_IF(USING(PLATFORM_WINDOWS))