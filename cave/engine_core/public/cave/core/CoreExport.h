// =============================================================================
// File: cave/core/CoreExport.h
// =============================================================================
#pragma once

#if defined(_WIN32)
#if defined(CAVE_CORE_BUILD_DLL)
#define CAVE_CORE_API __declspec(dllexport)
#else
#define CAVE_CORE_API __declspec(dllimport)
#endif
#else
#define CAVE_CORE_API
#endif