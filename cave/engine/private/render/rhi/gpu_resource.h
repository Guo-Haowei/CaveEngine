#pragma once
#include "cave/render/rhi/Defines.h"

#include "cave/render/rhi/PixelFormat.h"

namespace cave::render {

enum CpuAccessFlags {
    CPU_ACCESS_WRITE = 1,
    CPU_ACCESS_READ = 2,
};
DEFINE_ENUM_BITWISE_OPERATIONS(CpuAccessFlags)

// clang-format off
enum BindFlags : uint32_t {
    BIND_NONE             = 0x0,
    BIND_VERTEX_BUFFER    = 0x1,
    BIND_INDEX_BUFFER     = 0x2,
    BIND_CONSTANT_BUFFER  = 0x4,
    BIND_SHADER_RESOURCE  = 0x8,
    BIND_STREAM_OUTPUT    = 0x10,
    BIND_RENDER_TARGET    = 0x20,
    BIND_DEPTH_STENCIL    = 0x40,
    BIND_UNORDERED_ACCESS = 0x80,
    BIND_DECODER          = 0x100,
    BIND_VIDEO_ENCODER    = 0x200,
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(BindFlags)

// clang-format off
enum ResourceMiscFlags : uint32_t {
    RESOURCE_MISC_NONE                            = 0,
    RESOURCE_MISC_GENERATE_MIPS                   = BIT(0),
    RESOURCE_MISC_SHARED                          = BIT(1),
    RESOURCE_MISC_TEXTURECUBE                     = BIT(2),
    RESOURCE_MISC_DRAWINDIRECT_ARGS               = BIT(3),
    RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS          = BIT(4),
    RESOURCE_MISC_BUFFER_STRUCTURED               = BIT(5),
    RESOURCE_MISC_RESOURCE_CLAMP                  = BIT(6),
    RESOURCE_MISC_SHARED_KEYEDMUTEX               = BIT(7),
    RESOURCE_MISC_GDI_COMPATIBLE                  = BIT(8),
    RESOURCE_MISC_SHARED_NTHANDLE                 = BIT(9),
    RESOURCE_MISC_RESTRICTED_CONTENT              = BIT(10),
    RESOURCE_MISC_RESTRICT_SHARED_RESOURCE        = BIT(11),
    RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER = BIT(12),
    RESOURCE_MISC_GUARDED                         = BIT(13),
    RESOURCE_MISC_TILE_POOL                       = BIT(14),
    RESOURCE_MISC_TILED                           = BIT(15),
    RESOURCE_MISC_HW_PROTECTED                    = BIT(16),
    RESOURCE_MISC_SHARED_DISPLAYABLE              = BIT(17),
    RESOURCE_MISC_SHARED_EXCLUSIVE_WRITER         = BIT(18),
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(ResourceMiscFlags)

// @TODO: refactor
enum class AttachmentType {
    NONE = 0,
    COLOR_2D,
    COLOR_CUBE,
    DEPTH_2D,
    DEPTH_STENCIL_2D,
    SHADOW_2D,
    SHADOW_CUBE_ARRAY,
    RW_TEXTURE,
};

struct GpuTextureDesc {
    AttachmentType type;  // this should be removed

    Dimension dimension;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mip_levels;
    uint32_t array_size;
    PixelFormat format;
    BindFlags bind_flags;
    ResourceMiscFlags misc_flags;
    const void* initial_data = nullptr;
    std::string name; // @TODO: use char[]
};

struct GpuBufferDesc {
    GpuBufferType type{ GpuBufferType::Unknown };
    // @TODO: need better flags than this
    bool dynamic{ false };
    uint32_t slot{ 0 };  // remove this if possible
    uint32_t element_size{ 0 };
    uint32_t element_count{ 0 };
    const void* initial_data{ nullptr };
};

struct GpuBuffer {
    const GpuBufferDesc desc;

    GpuBuffer(const GpuBufferDesc& desc)
        : desc(desc) {}

    virtual ~GpuBuffer() = default;

    uint32_t capacity() const { return desc.element_count * desc.element_size; }

    int getSlot() const { return desc.slot; }

    virtual uint64_t getHandle() const = 0;
    uint32_t getHandle32() const { return static_cast<uint32_t>(getHandle()); }
};

constexpr int kMeshMaxVertexBufferCount = 8;

struct GpuMeshDesc {
    struct VertexLayout {
        uint32_t slot{ 0 };
        uint32_t stride_in_byte{ 0 };
        uint32_t offset_in_byte{ 0 };
    };

    uint32_t draw_count{ 0 };
    uint32_t enabled_vertex_count{ 0 };
    VertexLayout vertex_layout[kMeshMaxVertexBufferCount];
};

struct GpuMesh {
    const GpuMeshDesc desc;
    Ref<GpuBuffer> indexBuffer;
    std::array<Ref<GpuBuffer>, kMeshMaxVertexBufferCount> vertexBuffers;

    GpuMesh() = default;
    GpuMesh(const GpuMeshDesc& p_desc)
        : desc(p_desc) {}

    virtual ~GpuMesh() = default;
};

struct GpuTexture {
    GpuTexture(const GpuTextureDesc& p_desc)
        : desc(p_desc) {}

    virtual ~GpuTexture() = default;

    virtual uint64_t GetResidentHandle() const = 0;
    virtual uint64_t GetHandle() const = 0;
    virtual uint64_t GetUavHandle() const = 0;

    uint32_t GetHandle32() const { return static_cast<uint32_t>(GetHandle()); }

    const GpuTextureDesc desc;
};

// @TODO: use actual id
using GpuTextureId = std::shared_ptr<GpuTexture>;

}  // namespace cave::render
