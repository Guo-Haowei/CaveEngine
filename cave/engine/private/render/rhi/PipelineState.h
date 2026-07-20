#pragma once
#include "engine/private/renderer/graphics_defines.h"
#include "engine/private/renderer/pixel_format.h"

namespace cave {

struct InputLayoutDesc {
    struct Element {
        std::string semantic_name;
        uint32_t semantic_index;
        PixelFormat format;
        uint32_t input_slot;
        uint32_t aligned_byte_offset;
        InputClassification input_slot_class;
        uint32_t instance_data_step_rate;
    };

    std::vector<Element> elements;
};

struct RasterizerDesc {
    FillMode fillMode{ FillMode::SOLID };
    CullMode cullMode{ CullMode::BACK };
    bool frontCounterClockwise{ false };
    int depthBias{ 0 };
    float depthBiasClamp{ 0.0f };
    float slopeScaledDepthBias{ 0.0f };
    bool depthClipEnable{ false };
    bool scissorEnable{ false };
    bool multisampleEnable{ false };
    bool antialiasedLineEnable{ false };
};

constexpr uint8_t DEFAULT_STENCIL_READ_MASK = 0xff;
constexpr uint8_t DEFAULT_STENCIL_WRITE_MASK = 0xff;

struct StencilOpDesc {
    StencilOp stencilFailOp{ StencilOp::KEEP };
    StencilOp stencilDepthFailOp{ StencilOp::KEEP };
    StencilOp stencilPassOp{ StencilOp::KEEP };
    ComparisonFunc stencilFunc{ ComparisonFunc::ALWAYS };
};

struct DepthStencilDesc {
    bool depthEnabled;
    ComparisonFunc depthFunc;

    bool stencilEnabled;
    uint8_t stencilReadMask{ DEFAULT_STENCIL_READ_MASK };
    uint8_t stencilWriteMask{ DEFAULT_STENCIL_WRITE_MASK };
    StencilOpDesc frontFace{};
    StencilOpDesc backFace{};
};

struct ShaderMacro {
    const char* name;
    const char* value;
};

enum class PipelineStateType {
    GRAPHICS,
    COMPUTE,
};

struct PipelineStateDesc {
    PipelineStateType type{ PipelineStateType::GRAPHICS };

    std::string_view vs;
    std::string_view ps;
    std::string_view gs;
    std::string_view cs;

    PrimitiveTopology primitive_topology{ PrimitiveTopology::TRIANGLE };
    const RasterizerDesc* rasterizer_desc{};
    const DepthStencilDesc* depth_stencil_desc{};
    const InputLayoutDesc* input_layout_desc{};
    const BlendDesc* blend_desc{};

    uint32_t num_render_targets{ 0 };
    PixelFormat rtv_formats[8]{};
    PixelFormat dsv_format{};
};

struct PipelineState {
    PipelineState(const PipelineStateDesc& pso_desc)
        : desc(pso_desc) {}

    virtual ~PipelineState() = default;

    const PipelineStateDesc desc;
};

#define PSO_NAME_LIST                    \
    PSO_NAME(PSO_DPETH)                  \
    PSO_NAME(PSO_POINT_SHADOW)           \
    PSO_NAME(PSO_PREPASS)                \
    PSO_NAME(PSO_GBUFFER)                \
    PSO_NAME(PSO_GBUFFER_DOUBLE_SIDED)   \
    PSO_NAME(PSO_FORWARD_TRANSPARENT)    \
    PSO_NAME(PSO_VOXELIZATION)           \
    PSO_NAME(PSO_VOXELIZATION_PRE)       \
    PSO_NAME(PSO_VOXELIZATION_POST)      \
    PSO_NAME(PSO_HIGHLIGHT)              \
    PSO_NAME(PSO_LIGHTING)               \
    PSO_NAME(PSO_BLOOM_SETUP)            \
    PSO_NAME(PSO_BLOOM_DOWNSAMPLE)       \
    PSO_NAME(PSO_BLOOM_UPSAMPLE)         \
    PSO_NAME(PSO_SSAO)                   \
    PSO_NAME(PSO_POST_PROCESS)           \
    PSO_NAME(PSO_DEBUG_VOXEL)            \
    PSO_NAME(PSO_ENV_SKYBOX_TO_CUBE_MAP) \
    PSO_NAME(PSO_DIFFUSE_IRRADIANCE)     \
    PSO_NAME(PSO_PREFILTER)              \
    PSO_NAME(PSO_ENV_SKYBOX)             \
    PSO_NAME(PSO_BILLBOARD)              \
    PSO_NAME(PSO_PATH_TRACER)            \
    PSO_NAME(PSO_PARTICLE_INIT)          \
    PSO_NAME(PSO_PARTICLE_KICKOFF)       \
    PSO_NAME(PSO_PARTICLE_EMIT)          \
    PSO_NAME(PSO_PARTICLE_SIM)           \
    PSO_NAME(PSO_PARTICLE_RENDERING)     \
    PSO_NAME(PSO_UI_OVERLAY)             \
    PSO_NAME(PSO_PRIMITIVE)

enum PipelineStateName : uint8_t {
#define PSO_NAME(ENUM) ENUM,
    PSO_NAME_LIST
#undef PSO_NAME

        PSO_NAME_MAX,
};

static inline const char* EnumToString(PipelineStateName name) {
    DEV_ASSERT_INDEX(name, PipelineStateName::PSO_NAME_MAX);
    static const char* s_table[] = {
#define PSO_NAME(ENUM) #ENUM,
        PSO_NAME_LIST
#undef PSO_NAME
    };
    static_assert(std::size(s_table) == std::to_underlying(PipelineStateName::PSO_NAME_MAX));
    return s_table[name];
}

}  // namespace cave