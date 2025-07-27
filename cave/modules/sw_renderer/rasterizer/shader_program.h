#pragma once
#include "linalg.h"
#include "sampler.h"

namespace cave::rs {

enum VaryingFlag : unsigned int {
    VARYING_COLOR = 1u << 0,
    VARYING_NORMAL = 1u << 1,
    VARYING_UV = 1u << 2,
    VARYING_WORLD_POSITION = 1u << 3,
};

struct alignas(16) VSInput {
    Vector4f position;
    Vector4f normal;
    Vector4i boneId;
    Vector4f weights;
    Vector2f uv;
};

struct alignas(16) VSOutput {
    Vector4f position;
    Vector4f worldPosition;
    Vector4f normal;
    Vector4f color;
    Vector2f uv;
};

class IVertexShader {
public:
    virtual VSOutput processVertex(const VSInput& input) = 0;

    unsigned int getVaryingFlags() const { return m_varyingFlags; }

protected:
    IVertexShader() {}

    IVertexShader(unsigned int varyingFlags)
        : m_varyingFlags(varyingFlags) {}

    const unsigned int m_varyingFlags = 0;
};

class IFragmentShader {
public:
    virtual Color processFragment(const VSOutput& input) = 0;
};

}  // namespace cave::rs
