#pragma once
#include "cave/core/variant/Variant.h"

#define ENABLE_DVAR USE_IF(!USING(PLATFORM_WASM))

#if USING(ENABLE_DVAR)
// clang-format off
enum DvarFlags : uint32_t {
    DVAR_FLAG_NONE       = 0,
    DVAR_FLAG_CACHE      = 1,
    DVAR_FLAG_OVERRIDDEN = 2,
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(DvarFlags);

namespace cave {

class Dvar {
public:
    explicit Dvar(String name,
                  Variant&& variant,
                  DvarFlags flags,
                  const char* desc);

    bool setValue(Variant&& variant);

    template<typename... Args>
    bool setValue(Args&&... args) {
        return setValue(Variant{ std::forward<Args>(args)... });
    }

    void setFlag(DvarFlags flag) { m_flags |= flag; }
    void unsetFlag(DvarFlags flag) { m_flags &= ~flag; }

    Variant& variant() { return m_variant; }
    const Variant& variant() const { return m_variant; }

    const String& name() const { return m_name; }

    VariantType type() const { return m_variant.type(); }
    std::string_view desc() const { return m_desc; }
    uint32_t flags() const { return m_flags; }

private:
    String m_name;
    Variant m_variant;

    std::string_view m_desc;
    DvarFlags m_flags;
};

Dvar* FindStaticDvar(std::string_view name);
bool RegisterStaticDvar(Dvar* dvar);

}  // namespace cave

#define DVAR_GET_BOOL(name)    (DVAR_##name).variant().asBool()
#define DVAR_GET_INT(name)     (DVAR_##name).variant().asInt()
#define DVAR_GET_FLOAT(name)   (DVAR_##name).variant().asFloat()
#define DVAR_GET_STRING(name)  (DVAR_##name).variant().asString()
#define DVAR_GET_VEC2(name)    (DVAR_##name).variant().asVec2f()
#define DVAR_GET_VEC3(name)    (DVAR_##name).variant().asVec3f()
#define DVAR_GET_VEC4(name)    (DVAR_##name).variant().asVec4f()
#define DVAR_GET_IVEC2(name)   (DVAR_##name).variant().asVec2i()
#define DVAR_GET_IVEC3(name)   (DVAR_##name).variant().asVec3i()
#define DVAR_GET_IVEC4(name)   (DVAR_##name).variant().asVec4i()
#define DVAR_GET_POINTER(name) (DVAR_##name).variant().asPointer()

#define DVAR_SET_BOOL(name, value)       (DVAR_##name).setValue(!!(value))
#define DVAR_SET_INT(name, value)        (DVAR_##name).setValue(value)
#define DVAR_SET_FLOAT(name, value)      (DVAR_##name).setValue(value)
#define DVAR_SET_STRING(name, value)     (DVAR_##name).setValue(value)
#define DVAR_SET_VEC2(name, x, y)        (DVAR_##name).setValue(x, y)
#define DVAR_SET_VEC3(name, x, y, z)     (DVAR_##name).setValue(x, y, z)
#define DVAR_SET_VEC4(name, x, y, z, w)  (DVAR_##name).setValue(x, y, z, w)
#define DVAR_SET_IVEC2(name, x, y)       (DVAR_##name).setValue(x, y)
#define DVAR_SET_IVEC3(name, x, y, z)    (DVAR_##name).setValue(x, y, z)
#define DVAR_SET_IVEC4(name, x, y, z, w) (DVAR_##name).setValue(x, y, z, w)

#else

#define DVAR_GET_BOOL(name)    !!(DVAR_##name)
#define DVAR_GET_INT(name)     (DVAR_##name)
#define DVAR_GET_FLOAT(name)   (DVAR_##name)
#define DVAR_GET_STRING(name)  (DVAR_##name)
#define DVAR_GET_VEC2(name)    (DVAR_##name)
#define DVAR_GET_VEC3(name)    (DVAR_##name)
#define DVAR_GET_VEC4(name)    (DVAR_##name)
#define DVAR_GET_IVEC2(name)   (DVAR_##name)
#define DVAR_GET_IVEC3(name)   (DVAR_##name)
#define DVAR_GET_IVEC4(name)   (DVAR_##name)
#define DVAR_GET_POINTER(name) ((void*)&(DVAR_##name))

#define DVAR_SET_BOOL(name, value)       ((void)0)
#define DVAR_SET_INT(name, value)        ((void)0)
#define DVAR_SET_FLOAT(name, value)      ((void)0)
#define DVAR_SET_STRING(name, value)     ((void)0)
#define DVAR_SET_VEC2(name, x, y)        ((void)0)
#define DVAR_SET_VEC3(name, x, y, z)     ((void)0)
#define DVAR_SET_VEC4(name, x, y, z, w)  ((void)0)
#define DVAR_SET_IVEC2(name, x, y)       ((void)0)
#define DVAR_SET_IVEC3(name, x, y, z)    ((void)0)
#define DVAR_SET_IVEC4(name, x, y, z, w) ((void)0)

#endif
