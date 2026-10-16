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

struct GpuResourceDesc {
    Dimension dimension = Dimension::Texture2D;
    BufferUsage usage = BufferUsage::Default;

    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t depth = 1;
    uint32_t mip_levels = 1;
    uint32_t array_size = 1;

    PixelFormat format = PixelFormat::UNKNOWN;
    BindFlags bind_flags = BIND_NONE;
    ResourceMiscFlags misc_flags = RESOURCE_MISC_NONE;

    const void* initial_data = nullptr;
    std::string name;  // @TODO: use char[]

    uint32_t element_stride = 0;

    // @TODO: remove the following
    AttachmentType type = AttachmentType::NONE;
    uint32_t deprecated_slot = 0;

    bool isBuffer() const { return dimension == Dimension::Buffer; }
    bool isTexture() const { return dimension != Dimension::Buffer; }
    bool isStructured() const { return element_stride > 0; }
    bool isDynamic() const { return usage == BufferUsage::Dynamic; }
    uint32_t byteSize() const { return width; }
    uint32_t elementCount() const { return element_stride > 0 ? (width / element_stride) : 0; }

    static GpuResourceDesc vertexBuffer(uint32_t size, uint32_t max_count, const void* initial_data) {
        GpuResourceDesc desc{};
        desc.dimension = Dimension::Buffer;
        desc.width = size * max_count;
        desc.element_stride = size;  // @TODO: should not set this here
        desc.bind_flags = BIND_VERTEX_BUFFER;
        desc.initial_data = initial_data;
        return desc;
    }

    static GpuResourceDesc indexBuffer(uint32_t size, uint32_t max_count, const void* initial_data) {
        GpuResourceDesc desc{};
        desc.dimension = Dimension::Buffer;
        desc.width = size * max_count;
        desc.element_stride = size;  // @TODO: should not set this here
        desc.bind_flags = BIND_INDEX_BUFFER;
        desc.initial_data = initial_data;
        return desc;
    }

    static GpuResourceDesc constantBuffer(uint32_t size, uint32_t max_count) {
        GpuResourceDesc desc{};
        desc.dimension = Dimension::Buffer;
        desc.usage = BufferUsage::Dynamic;
        desc.width = size * max_count;
        desc.element_stride = size;  // @TODO: should not set this here
        desc.bind_flags = BIND_CONSTANT_BUFFER;
        return desc;
    }

    static GpuResourceDesc structuredBuffer(uint32_t element_stride, uint32_t element_count) {
        GpuResourceDesc desc{};
        desc.dimension = Dimension::Buffer;
        desc.usage = BufferUsage::Default;
        desc.width = element_stride * element_count;
        desc.element_stride = element_stride;
        desc.bind_flags = BIND_SHADER_RESOURCE;
        // desc.bind_flags |= BIND_UNORDERED_ACCESS;
        desc.misc_flags = RESOURCE_MISC_BUFFER_STRUCTURED;
        return desc;
    }
};

struct GpuBuffer {
    const GpuResourceDesc desc;

    GpuBuffer(const GpuResourceDesc& desc)
        : desc(desc) {}

    virtual ~GpuBuffer() = default;

    int getSlot() const { return desc.deprecated_slot; }

    virtual uint64_t getHandle() const = 0;
    uint32_t getHandle32() const { return static_cast<uint32_t>(getHandle()); }
};

struct GpuTexture {
    GpuTexture(const GpuResourceDesc& desc)
        : desc(desc) {}

    virtual ~GpuTexture() = default;

    virtual uint64_t getResidentHandle() const = 0;
    virtual uint64_t getHandle() const = 0;
    virtual uint64_t getUavHandle() const = 0;

    uint32_t GetHandle32() const { return static_cast<uint32_t>(getHandle()); }

    const GpuResourceDesc desc;
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

// @TODO: use actual id
using GpuTextureId = std::shared_ptr<GpuTexture>;

}  // namespace cave::render
