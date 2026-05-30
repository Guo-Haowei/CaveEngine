#pragma once
#include "engine/private/renderer/pixel_format.h"

namespace cave {

enum class BufferUsage {
    DEFAULT = 0,
    IMMUTABLE = 1,
    DYNAMIC = 2,
    STAGING = 3,
};

enum class Dimension : uint32_t {
    TEXTURE_1D,
    TEXTURE_1D_ARRAY,
    TEXTURE_2D,
    TEXTURE_2D_ARRAY,
    TEXTURE_CUBE,
    TEXTURE_CUBE_ARRAY,
    TEXTURE_3D,
};

enum CpuAccessFlags {
    CPU_ACCESS_WRITE = BIT(1),
    CPU_ACCESS_READ = BIT(2),
};
DEFINE_ENUM_BITWISE_OPERATIONS(CpuAccessFlags)

// clang-format off
enum BindFlags : uint32_t {
    BIND_NONE             = 0,
    BIND_VERTEX_BUFFER    = BIT(0),
    BIND_INDEX_BUFFER     = BIT(1),
    BIND_CONSTANT_BUFFER  = BIT(2),
    BIND_SHADER_RESOURCE  = BIT(3),
    BIND_STREAM_OUTPUT    = BIT(4),
    BIND_RENDER_TARGET    = BIT(5),
    BIND_DEPTH_STENCIL    = BIT(6),
    BIND_UNORDERED_ACCESS = BIT(7),
    BIND_DECODER          = BIT(8),
    BIND_VIDEO_ENCODER    = BIT(9),
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
    uint32_t mipLevels;
    uint32_t arraySize;
    PixelFormat format;
    BindFlags bindFlags;
    ResourceMiscFlags miscFlags;
    const void* initialData;
    // @TODO: get rid of the name
    std::string name;
};

enum class GpuBufferType : uint8_t {
    Unknown,
    Vertex,
    Index,
    Constant,
    Structured,
};

struct GpuBufferDesc {
    GpuBufferType type{ GpuBufferType::Unknown };
    // @TODO: need better flags than this
    bool dynamic{ false };
    uint32_t slot{ 0 };  // remove this if possible
    uint32_t element_size{ 0 };
    uint32_t element_count{ 0 };
    uint32_t offset{ 0 };
    const void* initial_data{ nullptr };
};

// @TODO: generalize buffers
struct GpuBuffer {
    const GpuBufferDesc desc;

    GpuBuffer(const GpuBufferDesc& p_desc)
        : desc(p_desc) {}

    virtual ~GpuBuffer() = default;

    virtual uint64_t GetHandle() const = 0;
    uint32_t GetHandle32() const { return static_cast<uint32_t>(GetHandle()); }
};

struct GpuConstantBuffer {
    GpuConstantBuffer(const GpuBufferDesc& p_desc)
        : desc(p_desc), capacity(desc.element_count * desc.element_size) {}

    virtual ~GpuConstantBuffer() = default;

    int GetSlot() const { return desc.slot; }

    const GpuBufferDesc desc;
    // @TODO: refactor this
    const uint32_t capacity;
};

struct GpuStructuredBuffer {
    const GpuBufferDesc desc;

    GpuStructuredBuffer(const GpuBufferDesc& p_desc)
        : desc(p_desc) {}

    virtual ~GpuStructuredBuffer() = default;
};

constexpr inline int MESH_MAX_VERTEX_BUFFER_COUNT = 8;

struct GpuMeshDesc {
    struct VertexLayout {
        uint32_t slot{ 0 };
        uint32_t strideInByte{ 0 };
        uint32_t offsetInByte{ 0 };
    };

    uint32_t drawCount{ 0 };  // draw count
    uint32_t enabledVertexCount{ 0 };
    VertexLayout vertexLayout[MESH_MAX_VERTEX_BUFFER_COUNT];
};

struct GpuMesh {
    const GpuMeshDesc desc;
    std::shared_ptr<GpuBuffer> indexBuffer;
    std::array<std::shared_ptr<GpuBuffer>, MESH_MAX_VERTEX_BUFFER_COUNT> vertexBuffers;

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

// @TODO: remove this
template<typename T>
struct ConstantBuffer {
    std::shared_ptr<GpuConstantBuffer> buffer;
    T cache;
};

}  // namespace cave
