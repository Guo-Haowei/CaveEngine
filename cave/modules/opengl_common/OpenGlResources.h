#ifdef OPENGL_RESOURCES_INCLUDED
#error DO NOT INCLUDE THIS FILE IN HEADER
#endif
#define OPENGL_RESOURCES_INCLUDED

#include "engine/private/render/rhi/RenderTarget.h"
#include "engine/private/renderer/gpu_resource.h"
#include "OpenGlDefines.h"

namespace cave {

struct OpenGlBuffer : GpuBuffer {
    using GpuBuffer::GpuBuffer;

    gl::BUFFER_TYPE type;
    uint32_t handle{ 0 };

    ~OpenGlBuffer() { Clear(); }

    void Clear();

    uint64_t GetHandle() const final { return handle; }
};

struct OpenGlGpuTexture : public GpuTexture {
    using GpuTexture::GpuTexture;

    ~OpenGlGpuTexture() { Clear(); }

    void Clear();

    uint64_t GetResidentHandle() const final {
        return residentHandle;
    }
    uint64_t GetHandle() const final { return handle; }
    uint64_t GetUavHandle() const final { return handle; }

    uint32_t handle = 0;
    uint64_t residentHandle = 0;
};

struct OpenGlConstantBuffer : public GpuConstantBuffer {
    using GpuConstantBuffer::GpuConstantBuffer;

    ~OpenGlConstantBuffer() { clear(); }

    void clear();

    uint32_t handle = 0;
};

struct OpenGlStructuredBuffer : public GpuStructuredBuffer {
    using GpuStructuredBuffer::GpuStructuredBuffer;

    ~OpenGlStructuredBuffer() { clear(); }

    void clear();

    uint32_t handle = 0;
};

}  // namespace cave
