#pragma once
#include "cave/core/math/Vector.h"

#define ENABLE_DVAR USE_IF(!USING(PLATFORM_WASM))

#if USING(ENABLE_DVAR)
// clang-format off
enum DvarFlags : uint32_t {
    DVAR_FLAG_NONE      = 0,
    DVAR_FLAG_CACHE     = BIT(0),
    DVAR_FLAG_OVERRIDEN = BIT(1),
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(DvarFlags);

namespace cave {

enum VariantType {
    VARIANT_TYPE_INVALID = 0,
    VARIANT_TYPE_INT,
    VARIANT_TYPE_FLOAT,
    VARIANT_TYPE_STRING,
    VARIANT_TYPE_VEC2,
    VARIANT_TYPE_VEC3,
    VARIANT_TYPE_VEC4,
    VARIANT_TYPE_IVEC2,
    VARIANT_TYPE_IVEC3,
    VARIANT_TYPE_IVEC4,
    VARIANT_TYPE_MAX,
};

class Dvar {
public:
    explicit Dvar(VariantType type, DvarFlags flags, const char* desc);

    void RegisterInt(std::string_view key, int value);
    void RegisterFloat(std::string_view key, float value);
    void RegisterString(std::string_view key, std::string_view value);
    void RegisterVector2f(std::string_view key, float x, float y);
    void RegisterVector3f(std::string_view key, float x, float y, float z);
    void RegisterVector4f(std::string_view key, float x, float y, float z, float w);
    void RegisterVector2i(std::string_view key, int x, int y);
    void RegisterVector3i(std::string_view key, int x, int y, int z);
    void RegisterVector4i(std::string_view key, int x, int y, int z, int w);

    [[nodiscard]] int AsInt() const;
    [[nodiscard]] float AsFloat() const;
    [[nodiscard]] const std::string& AsString() const;
    [[nodiscard]] math::Vec2f asVec2f() const;
    [[nodiscard]] math::Vec3f asVec3f() const;
    [[nodiscard]] math::Vec4f asVec4f() const;
    [[nodiscard]] math::Vec2i asVec2i() const;
    [[nodiscard]] math::Vec3i asVec3i() const;
    [[nodiscard]] math::Vec4i asVec4i() const;
    [[nodiscard]] void* AsPointer();

    bool SetInt(int value);
    bool SetFloat(float value);
    bool SetString(const std::string& value);
    bool SetString(std::string_view value);
    bool SetVector2f(float x, float y);
    bool SetVector3f(float x, float y, float z);
    bool SetVector4f(float x, float y, float z, float w);
    bool SetVector2i(int x, int y);
    bool SetVector3i(int x, int y, int z);
    bool SetVector4i(int x, int y, int z, int w);

    void SetFlag(DvarFlags flag) { m_flags |= flag; }
    void UnsetFlag(DvarFlags flag) { m_flags &= ~flag; }

    std::string ValueToString() const;

    VariantType GetType() const { return m_type; }
    const char* GetDesc() const { return m_desc; }
    uint32_t GetFlags() const { return m_flags; }

    static Dvar* FindDvar(const std::string& name);
    static void RegisterDvar(std::string_view key, Dvar* dvar);

private:
    const VariantType m_type;
    const char* m_desc;
    DvarFlags m_flags;

    union {
        int m_int;
        float m_float;
        math::Vec4f m_vec;
        math::Vec4i m_ivec;
    };
    std::string m_string;
    std::string m_name;

    inline static std::unordered_map<std::string, Dvar*> s_map;
    friend class DvarCache;
    friend class RegisterCommands;
};

}  // namespace cave

#define DVAR_GET_BOOL(name)    (!!(DVAR_##name).AsInt())
#define DVAR_GET_INT(name)     (DVAR_##name).AsInt()
#define DVAR_GET_FLOAT(name)   (DVAR_##name).AsFloat()
#define DVAR_GET_STRING(name)  (DVAR_##name).AsString()
#define DVAR_GET_VEC2(name)    (DVAR_##name).asVec2f()
#define DVAR_GET_VEC3(name)    (DVAR_##name).asVec3f()
#define DVAR_GET_VEC4(name)    (DVAR_##name).asVec4f()
#define DVAR_GET_IVEC2(name)   (DVAR_##name).asVec2i()
#define DVAR_GET_IVEC3(name)   (DVAR_##name).asVec3i()
#define DVAR_GET_IVEC4(name)   (DVAR_##name).asVec4i()
#define DVAR_GET_POINTER(name) (DVAR_##name).AsPointer()

#define DVAR_SET_BOOL(name, value)       (DVAR_##name).SetInt(!!(value))
#define DVAR_SET_INT(name, value)        (DVAR_##name).SetInt(value)
#define DVAR_SET_FLOAT(name, value)      (DVAR_##name).SetFloat(value)
#define DVAR_SET_STRING(name, value)     (DVAR_##name).SetString(value)
#define DVAR_SET_VEC2(name, x, y)        (DVAR_##name).SetVector2f(x, y)
#define DVAR_SET_VEC3(name, x, y, z)     (DVAR_##name).SetVector3f(x, y, z)
#define DVAR_SET_VEC4(name, x, y, z, w)  (DVAR_##name).SetVector4f(x, y, z, w)
#define DVAR_SET_IVEC2(name, x, y)       (DVAR_##name).SetVector2i(x, y)
#define DVAR_SET_IVEC3(name, x, y, z)    (DVAR_##name).SetVector3i(x, y, z)
#define DVAR_SET_IVEC4(name, x, y, z, w) (DVAR_##name).SetVector4i(x, y, z, w)

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
