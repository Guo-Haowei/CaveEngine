#pragma once

#if USING(PLATFORM_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>
#endif

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif
