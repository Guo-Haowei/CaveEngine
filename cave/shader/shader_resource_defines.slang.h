/// File: shader_resource_defines.slang.h
#pragma once

#define SRV_DEFINES              \
    TEXTURE_2D(BaseColorMap, 20) \
    TEXTURE_2D(NormalMap, 21)    \
    TEXTURE_2D(MaterialMap, 22)

#if defined(__SLANG__)
#ifndef TEXTURE_2D
#error "include header"
#endif
SRV_DEFINES
#elif defined(__cplusplus)
#ifdef TEXTURE_2D
#error "undef TEXTURE_2D"
#endif
#define TEXTURE_2D(NAME, SLOT) \
    [[maybe_unused]] static constexpr inline int Get##NAME##Slot() { return SLOT; }
SRV_DEFINES
#undef TEXTURE_2D
#else
#error Platform not supported
#endif
