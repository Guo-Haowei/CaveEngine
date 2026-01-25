// =============================================================================
// File: sdk/cave/Export.h
// =============================================================================
#pragma once

// Detect platform
#if defined(_WIN32) || defined(_WIN64)
#define CAVE_PLATFORM_WINDOWS 1
#else
#define CAVE_PLATFORM_WINDOWS 0
#endif

// Shared library import/export
#if CAVE_PLATFORM_WINDOWS
#if defined(CAVE_GAME_DLL)  // defined when building game.dll
#define CAVE_API __declspec(dllexport)
#else  // used by host/editor/runtime
#define CAVE_API __declspec(dllimport)
#endif
#else
// GCC / Clang visibility
#if __GNUC__ >= 4
#define CAVE_API __attribute__((visibility("default")))
#else
#define CAVE_API
#endif
#endif