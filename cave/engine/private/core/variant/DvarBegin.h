#include "Dvar.h"

#if USING(ENABLE_DVAR)

#if defined(DEFINE_DVAR)
#define DVAR_BOOL(name, flags, desc, value)       cave::Dvar DVAR_##name(name, cave::Variant(value), flags, desc)
#define DVAR_INT(name, flags, desc, value)        cave::Dvar DVAR_##name(name, cave::Variant(value), flags, desc)
#define DVAR_FLOAT(name, flags, desc, value)      cave::Dvar DVAR_##name(name, cave::Variant(value), flags, desc)
#define DVAR_STRING(name, flags, desc, value)     cave::Dvar DVAR_##name(name, cave::Variant(value), flags, desc)
#define DVAR_VEC2(name, flags, desc, x, y)        cave::Dvar DVAR_##name(name, cave::Variant(x, y), flags, desc)
#define DVAR_VEC3(name, flags, desc, x, y, z)     cave::Dvar DVAR_##name(name, cave::Variant(x, y, z), flags, desc)
#define DVAR_VEC4(name, flags, desc, x, y, z, w)  cave::Dvar DVAR_##name(name, cave::Variant(x, y, z, w), flags, desc)
#define DVAR_IVEC2(name, flags, desc, x, y)       cave::Dvar DVAR_##name(name, cave::Variant(x, y), flags, desc)
#define DVAR_IVEC3(name, flags, desc, x, y, z)    cave::Dvar DVAR_##name(name, cave::Variant(x, y, z), flags, desc)
#define DVAR_IVEC4(name, flags, desc, x, y, z, w) cave::Dvar DVAR_##name(name, cave::Variant(x, y, z, w), flags, desc)
#endif

#if defined(REGISTER_DVAR)
#define DVAR_BOOL(name, ...)   cave::RegisterStatic(DVAR_##name)
#define DVAR_INT(name, ...)    cave::RegisterStatic(DVAR_##name)
#define DVAR_FLOAT(name, ...)  cave::RegisterStatic(DVAR_##name)
#define DVAR_STRING(name, ...) cave::RegisterStatic(DVAR_##name)
#define DVAR_VEC2(name, ...)   cave::RegisterStatic(DVAR_##name)
#define DVAR_VEC3(name, ...)   cave::RegisterStatic(DVAR_##name)
#define DVAR_VEC4(name, ...)   cave::RegisterStatic(DVAR_##name)
#define DVAR_IVEC2(name, ...)  cave::RegisterStatic(DVAR_##name)
#define DVAR_IVEC3(name, ...)  cave::RegisterStatic(DVAR_##name)
#define DVAR_IVEC4(name, ...)  cave::RegisterStatic(DVAR_##name)
#endif

#if !defined(DEFINE_DVAR) && !defined(REGISTER_DVAR)
#define DVAR_BOOL(name, ...)   extern cave::Dvar DVAR_##name
#define DVAR_INT(name, ...)    extern cave::Dvar DVAR_##name
#define DVAR_FLOAT(name, ...)  extern cave::Dvar DVAR_##name
#define DVAR_STRING(name, ...) extern cave::Dvar DVAR_##name
#define DVAR_VEC2(name, ...)   extern cave::Dvar DVAR_##name
#define DVAR_VEC3(name, ...)   extern cave::Dvar DVAR_##name
#define DVAR_VEC4(name, ...)   extern cave::Dvar DVAR_##name
#define DVAR_IVEC2(name, ...)  extern cave::Dvar DVAR_##name
#define DVAR_IVEC3(name, ...)  extern cave::Dvar DVAR_##name
#define DVAR_IVEC4(name, ...)  extern cave::Dvar DVAR_##name
#endif

#else
#if defined(DEFINE_DVAR)
#define DVAR_BOOL(name, flags, desc, value)       bool DVAR_##name = { value }
#define DVAR_INT(name, flags, desc, value)        int DVAR_##name = { value }
#define DVAR_FLOAT(name, flags, desc, value)      float DVAR_##name = { value }
#define DVAR_STRING(name, flags, desc, value)     std::string DVAR_##name = { value }
#define DVAR_VEC2(name, flags, desc, x, y)        cave::Vec2f DVAR_##name = { x, y }
#define DVAR_VEC3(name, flags, desc, x, y, z)     cave::Vec3f DVAR_##name = { x, y, z }
#define DVAR_VEC4(name, flags, desc, x, y, z, w)  cave::Vec4f DVAR_##name = { x, y, z, w }
#define DVAR_IVEC2(name, flags, desc, x, y)       cave::Vec2i DVAR_##name = { x, y }
#define DVAR_IVEC3(name, flags, desc, x, y, z)    cave::Vec3i DVAR_##name = { x, y, z }
#define DVAR_IVEC4(name, flags, desc, x, y, z, w) cave::Vec4i DVAR_##name = { x, y, z, w }
#endif

#if defined(REGISTER_DVAR)
#define DVAR_BOOL(name, flags, desc, value)       ((void)0)
#define DVAR_INT(name, flags, desc, value)        ((void)0)
#define DVAR_FLOAT(name, flags, desc, value)      ((void)0)
#define DVAR_STRING(name, flags, desc, value)     ((void)0)
#define DVAR_VEC2(name, flags, desc, x, y)        ((void)0)
#define DVAR_VEC3(name, flags, desc, x, y, z)     ((void)0)
#define DVAR_VEC4(name, flags, desc, x, y, z, w)  ((void)0)
#define DVAR_IVEC2(name, flags, desc, x, y)       ((void)0)
#define DVAR_IVEC3(name, flags, desc, x, y, z)    ((void)0)
#define DVAR_IVEC4(name, flags, desc, x, y, z, w) ((void)0)
#endif

#if !defined(DEFINE_DVAR) && !defined(REGISTER_DVAR)
#define DVAR_BOOL(name, ...)   extern int DVAR_##name
#define DVAR_INT(name, ...)    extern int DVAR_##name
#define DVAR_FLOAT(name, ...)  extern float DVAR_##name
#define DVAR_STRING(name, ...) extern std::string DVAR_##name
#define DVAR_VEC2(name, ...)   extern ::cave::Vec2f DVAR_##name
#define DVAR_VEC3(name, ...)   extern ::cave::Vec3f DVAR_##name
#define DVAR_VEC4(name, ...)   extern ::cave::Vec4f DVAR_##name
#define DVAR_IVEC2(name, ...)  extern ::cave::Vec2i DVAR_##name
#define DVAR_IVEC3(name, ...)  extern ::cave::Vec3i DVAR_##name
#define DVAR_IVEC4(name, ...)  extern ::cave::Vec4i DVAR_##name
#endif

#endif
