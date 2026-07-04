#include "Dvar.h"

#if USING(ENABLE_DVAR)

#if defined(DEFINE_DVAR)
#define DVAR_BOOL(name, flags, desc, value)       cave::Dvar DVAR_##name(cave::VariantType::Int, flags, desc)
#define DVAR_INT(name, flags, desc, value)        cave::Dvar DVAR_##name(cave::VariantType::Int, flags, desc)
#define DVAR_FLOAT(name, flags, desc, value)      cave::Dvar DVAR_##name(cave::VariantType::Float, flags, desc)
#define DVAR_STRING(name, flags, desc, value)     cave::Dvar DVAR_##name(cave::VariantType::String, flags, desc)
#define DVAR_VEC2(name, flags, desc, x, y)        cave::Dvar DVAR_##name(cave::VariantType::Vec2f, flags, desc)
#define DVAR_VEC3(name, flags, desc, x, y, z)     cave::Dvar DVAR_##name(cave::VariantType::Vec3f, flags, desc)
#define DVAR_VEC4(name, flags, desc, x, y, z, w)  cave::Dvar DVAR_##name(cave::VariantType::Vec4f, flags, desc)
#define DVAR_IVEC2(name, flags, desc, x, y)       cave::Dvar DVAR_##name(cave::VariantType::Vec2i, flags, desc)
#define DVAR_IVEC3(name, flags, desc, x, y, z)    cave::Dvar DVAR_##name(cave::VariantType::Vec3i, flags, desc)
#define DVAR_IVEC4(name, flags, desc, x, y, z, w) cave::Dvar DVAR_##name(cave::VariantType::Vec4i, flags, desc)
#endif

#if defined(REGISTER_DVAR)
#define DVAR_BOOL(name, flags, desc, value)       (DVAR_##name).RegisterInt(#name, !!(value))
#define DVAR_INT(name, flags, desc, value)        (DVAR_##name).RegisterInt(#name, value)
#define DVAR_FLOAT(name, flags, desc, value)      (DVAR_##name).RegisterFloat(#name, value)
#define DVAR_STRING(name, flags, desc, value)     (DVAR_##name).RegisterString(#name, value)
#define DVAR_VEC2(name, flags, desc, x, y)        (DVAR_##name).RegisterVector2f(#name, x, y)
#define DVAR_VEC3(name, flags, desc, x, y, z)     (DVAR_##name).RegisterVector3f(#name, x, y, z)
#define DVAR_VEC4(name, flags, desc, x, y, z, w)  (DVAR_##name).RegisterVector4f(#name, x, y, z, w)
#define DVAR_IVEC2(name, flags, desc, x, y)       (DVAR_##name).RegisterVector2i(#name, x, y)
#define DVAR_IVEC3(name, flags, desc, x, y, z)    (DVAR_##name).RegisterVector3i(#name, x, y, z)
#define DVAR_IVEC4(name, flags, desc, x, y, z, w) (DVAR_##name).RegisterVector4i(#name, x, y, z, w)
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
#define DVAR_BOOL(name, flags, desc, value)       int DVAR_##name = { value }
#define DVAR_INT(name, flags, desc, value)        int DVAR_##name = { value }
#define DVAR_FLOAT(name, flags, desc, value)      float DVAR_##name = { value }
#define DVAR_STRING(name, flags, desc, value)     std::string DVAR_##name = { value }
#define DVAR_VEC2(name, flags, desc, x, y)        ::cave::Vector2f DVAR_##name = { x, y }
#define DVAR_VEC3(name, flags, desc, x, y, z)     ::cave::Vector3f DVAR_##name = { x, y, z }
#define DVAR_VEC4(name, flags, desc, x, y, z, w)  ::cave::Vector4f DVAR_##name = { x, y, z, w }
#define DVAR_IVEC2(name, flags, desc, x, y)       ::cave::Vector2i DVAR_##name = { x, y }
#define DVAR_IVEC3(name, flags, desc, x, y, z)    ::cave::Vector3i DVAR_##name = { x, y, z }
#define DVAR_IVEC4(name, flags, desc, x, y, z, w) ::cave::Vector4i DVAR_##name = { x, y, z, w }
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
#define DVAR_BOOL(name, ...)   extern int DVAR_##name
#define DVAR_INT(name, ...)    extern int DVAR_##name
#define DVAR_FLOAT(name, ...)  extern float DVAR_##name
#define DVAR_STRING(name, ...) extern std::string DVAR_##name
#define DVAR_VEC2(name, ...)   extern ::cave::Vector2f DVAR_##name
#define DVAR_VEC3(name, ...)   extern ::cave::Vector3f DVAR_##name
#define DVAR_VEC4(name, ...)   extern ::cave::Vector4f DVAR_##name
#define DVAR_IVEC2(name, ...)  extern ::cave::Vector2i DVAR_##name
#define DVAR_IVEC3(name, ...)  extern ::cave::Vector3i DVAR_##name
#define DVAR_IVEC4(name, ...)  extern ::cave::Vector4i DVAR_##name
#endif

#endif
