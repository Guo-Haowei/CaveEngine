// =============================================================================
// File: cave/render/rhi/PipelineState.h
// =============================================================================
#pragma once
#include "cave/render/rhi/Defines.h"
#include "cave/render/rhi/PixelFormat.h"

namespace cave::render {

struct InputLayoutDesc {
    struct Element {
        String semantic_name;
        uint32_t semantic_index;
        PixelFormat format;
        uint32_t input_slot;
        uint32_t aligned_byte_offset;
        InputClassification input_slot_class;
        uint32_t instance_data_step_rate;
    };

    Vector<Element> elements;
};

struct RasterizerDesc {
    FillMode fillMode{ FillMode::Solid };
    CullMode cullMode{ CullMode::Back };
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

    PrimitiveTopology primitive_topology{ PrimitiveTopology::Triangle };
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

#define PSO_NAME_LIST                \
    PSO_NAME(UiOverlay)              \
    PSO_NAME(Primitive)              \
    PSO_NAME(Depth)                  \
    PSO_NAME(PSO_POINT_SHADOW)       \
    PSO_NAME(Prepass)                \
    PSO_NAME(Gbuffer)                \
    PSO_NAME(GbufferDoubleSided)     \
    PSO_NAME(Forward)                \
    PSO_NAME(Highlight)              \
    PSO_NAME(Lighting)               \
    PSO_NAME(BloomSetup)             \
    PSO_NAME(BloomDownsample)        \
    PSO_NAME(BloomUpsample)          \
    PSO_NAME(Ssao)                   \
    PSO_NAME(PostProcess)            \
    PSO_NAME(PSO_DEBUG_VOXEL)        \
    PSO_NAME(EnvSkyboxToCubemap)     \
    PSO_NAME(DiffuseIrradiance)      \
    PSO_NAME(Prefilter)              \
    PSO_NAME(EnvSkybox)              \
    PSO_NAME(PathTracer)             \
    PSO_NAME(PSO_PARTICLE_INIT)      \
    PSO_NAME(PSO_PARTICLE_KICKOFF)   \
    PSO_NAME(PSO_PARTICLE_EMIT)      \
    PSO_NAME(PSO_PARTICLE_SIM)       \
    PSO_NAME(PSO_PARTICLE_RENDERING) \
    PSO_NAME(PSO_VOXELIZATION)       \
    PSO_NAME(PSO_VOXELIZATION_PRE)   \
    PSO_NAME(PSO_VOXELIZATION_POST)

enum class PipelineStateName : uint8_t {
#define PSO_NAME(ENUM) ENUM,
    PSO_NAME_LIST
#undef PSO_NAME

        Count,
};

static inline const char* EnumToString(PipelineStateName name) {
    DEV_ASSERT_INDEX(name, PipelineStateName::Count);
    static const char* s_table[] = {
#define PSO_NAME(ENUM) #ENUM,
        PSO_NAME_LIST
#undef PSO_NAME
    };
    static_assert(std::size(s_table) == std::to_underlying(PipelineStateName::Count));
    return s_table[std::to_underlying(name)];
}

}  // namespace cave::render