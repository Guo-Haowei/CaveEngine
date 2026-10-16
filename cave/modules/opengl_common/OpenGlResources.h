#ifdef OPENGL_RESOURCES_INCLUDED
#error DO NOT INCLUDE THIS FILE IN HEADER
#endif
#define OPENGL_RESOURCES_INCLUDED

#include "engine/private/render/rhi/gpu_resource.h"
#include "OpenGlDefines.h"

namespace cave::render {

struct OpenGlGpuMesh : GpuMesh {
    using GpuMesh::GpuMesh;

    uint32_t vao{ 0 };
};

struct OpenGlBuffer : GpuBuffer {
    using GpuBuffer::GpuBuffer;

    gl::BUFFER_TYPE type;
    uint32_t handle{ 0 };

    ~OpenGlBuffer() { clear(); }

    void clear();

    uint64_t getHandle() const final { return handle; }
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

}  // namespace cave::render
