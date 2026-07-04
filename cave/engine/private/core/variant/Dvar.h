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
    explicit Dvar(VariantType type, DvarFlags flags, const char* desc);

    void registerInt(std::string_view key, int value);
    void registerFloat(std::string_view key, float value);
    void registerString(std::string_view key, std::string_view value);
    void registerVector2f(std::string_view key, float x, float y);
    void registerVector3f(std::string_view key, float x, float y, float z);
    void registerVector4f(std::string_view key, float x, float y, float z, float w);
    void registerVector2i(std::string_view key, int x, int y);
    void registerVector3i(std::string_view key, int x, int y, int z);
    void registerVector4i(std::string_view key, int x, int y, int z, int w);

    bool setInt(int value);
    bool setFloat(float value);
    bool setString(const std::string& value);
    bool setString(std::string_view value);
    bool setVec2f(float x, float y);
    bool setVec3f(float x, float y, float z);
    bool setVec4f(float x, float y, float z, float w);
    bool setVec2i(int x, int y);
    bool setVec3i(int x, int y, int z);
    bool setVec4i(int x, int y, int z, int w);

    void setFlag(DvarFlags flag) { flags_ |= flag; }
    void unsetFlag(DvarFlags flag) { flags_ &= ~flag; }

    Variant& variant() { return variant_; }
    const Variant& variant() const { return variant_; }

    VariantType type() const { return variant_.type(); }
    const char* desc() const { return desc_; }
    uint32_t flags() const { return flags_; }

    static Dvar* findDvar(const std::string& name);
    static void registerDvar(std::string_view key, Dvar* dvar);

private:
    Variant variant_;

    const char* desc_;
    DvarFlags flags_;

    std::string name_;

    inline static std::unordered_map<std::string, Dvar*> s_map;
    friend class DvarCache;
    friend class RegisterCommands;
};

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

#define DVAR_SET_BOOL(name, value)       (DVAR_##name).setInt(!!(value))
#define DVAR_SET_INT(name, value)        (DVAR_##name).setInt(value)
#define DVAR_SET_FLOAT(name, value)      (DVAR_##name).setFloat(value)
#define DVAR_SET_STRING(name, value)     (DVAR_##name).setString(value)
#define DVAR_SET_VEC2(name, x, y)        (DVAR_##name).setVec2f(x, y)
#define DVAR_SET_VEC3(name, x, y, z)     (DVAR_##name).setVec3f(x, y, z)
#define DVAR_SET_VEC4(name, x, y, z, w)  (DVAR_##name).setVec4f(x, y, z, w)
#define DVAR_SET_IVEC2(name, x, y)       (DVAR_##name).setVec2i(x, y)
#define DVAR_SET_IVEC3(name, x, y, z)    (DVAR_##name).setVec3i(x, y, z)
#define DVAR_SET_IVEC4(name, x, y, z, w) (DVAR_##name).setVec4i(x, y, z, w)

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
