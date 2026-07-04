#pragma once
#include "PipelineState.h"

namespace cave {

// @TODO: make these class members
/// input layouts
static const InputLayoutDesc s_input_layout_mesh = {
    .elements = {
        { "POSITION", 0, PixelFormat::R32G32B32_FLOAT, 0, 0, InputClassification::PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, PixelFormat::R32G32B32_FLOAT, 1, 0, InputClassification::PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, PixelFormat::R32G32_FLOAT, 2, 0, InputClassification::PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, PixelFormat::R32G32B32_FLOAT, 3, 0, InputClassification::PER_VERTEX_DATA, 0 },
        { "BONEINDEX", 0, PixelFormat::R32G32B32A32_SINT, 4, 0, InputClassification::PER_VERTEX_DATA, 0 },
        { "BONEWEIGHT", 0, PixelFormat::R32G32B32A32_FLOAT, 5, 0, InputClassification::PER_VERTEX_DATA, 0 },
        { "COLOR", 0, PixelFormat::R32G32B32A32_FLOAT, 6, 0, InputClassification::PER_VERTEX_DATA, 0 },
    }
};

static const InputLayoutDesc s_input_layout_sprite = {
    .elements = {
        { "POSITION", 0, PixelFormat::R32G32_FLOAT, 0, 0, InputClassification::PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, PixelFormat::R32G32_FLOAT, 1, 0, InputClassification::PER_VERTEX_DATA, 0 },
    }
};

static const InputLayoutDesc s_input_layout_ui = {
    .elements = {
        { "POSITION", 0, PixelFormat::R32G32_FLOAT, 0, 0, InputClassification::PER_VERTEX_DATA, 0 },
        { "COLOR", 0, PixelFormat::R32G32B32A32_FLOAT, 1, 0, InputClassification::PER_VERTEX_DATA, 0 },
    }
};

static const InputLayoutDesc s_input_layout_position = {
    .elements = {
        { "POSITION", 0, PixelFormat::R32G32B32_FLOAT, 0, 0, InputClassification::PER_VERTEX_DATA, 0 },
    }
};

static const InputLayoutDesc s_input_layout_debug = {
    .elements = {
        { "POSITION", 0, PixelFormat::R32G32B32_FLOAT, 0, 0, InputClassification::PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, PixelFormat::R32G32_FLOAT, 1, 0, InputClassification::PER_VERTEX_DATA, 0 },
        { "COLOR", 0, PixelFormat::R32G32B32A32_FLOAT, 2, 0, InputClassification::PER_VERTEX_DATA, 0 },
    }
};

/// rasterizer states
static const RasterizerDesc s_rasterizer_cull_back = {
    .fillMode = FillMode::SOLID,
    .cullMode = CullMode::BACK,
    .frontCounterClockwise = true,
};

static const RasterizerDesc s_rasterizer_cull_front = {
    .fillMode = FillMode::SOLID,
    .cullMode = CullMode::FRONT,
    .frontCounterClockwise = true,
};

static const RasterizerDesc s_rasterizer_double_sided = {
    .fillMode = FillMode::SOLID,
    .cullMode = CullMode::NONE,
    .frontCounterClockwise = true,
};

/// Depth stencil states
static const DepthStencilDesc s_default_depth_stencil = {
    .depthEnabled = true,
    .depthFunc = ComparisonFunc::LESS_EQUAL,
    .stencilEnabled = false,
};

static const DepthStencilDesc s_depth_stencil_off = {
    .depthEnabled = false,
    .depthFunc = ComparisonFunc::ALWAYS,
    .stencilEnabled = false,
};

static const DepthStencilDesc s_depth_reversed_stencil_off = {
    .depthEnabled = true,
    .depthFunc = ComparisonFunc::GREATER_EQUAL,
    .stencilEnabled = false,
};

static const DepthStencilDesc s_depth_reversed_stencil_on = {
    .depthEnabled = true,
    .depthFunc = ComparisonFunc::GREATER_EQUAL,
    .stencilEnabled = true,
    .frontFace = {
        .stencilPassOp = StencilOp::REPLACE,
        .stencilFunc = ComparisonFunc::ALWAYS,
    },
};

static const DepthStencilDesc s_skybox_depth_stencil = {
    .depthEnabled = false,
    .depthFunc = ComparisonFunc::ALWAYS,
    .stencilEnabled = true,
    .frontFace = {
        .stencilFunc = ComparisonFunc::EQUAL,
    },
};

static const DepthStencilDesc s_depth_reversed_stencil_on_highlight = {
    .depthEnabled = false,
    .depthFunc = ComparisonFunc::ALWAYS,
    .stencilEnabled = true,
    .frontFace = {
        .stencilFunc = ComparisonFunc::EQUAL,
    },
};

/// Blend states
static const BlendDesc s_default_blend_state = {};

static const BlendDesc s_transparent = {
    .renderTargets = {
        {
            .blendEnabled = true,
            .blendSrc = Blend::BLEND_SRC_ALPHA,
            .blendDest = Blend::BLEND_INV_SRC_ALPHA,
            .blendOp = BlendOp::BLEND_OP_ADD,
        } }
};

static const BlendDesc s_blend_state_off = {
    .renderTargets = {
        { .colorWriteMask = COLOR_WRITE_ENABLE_NONE },
        { .colorWriteMask = COLOR_WRITE_ENABLE_NONE },
        { .colorWriteMask = COLOR_WRITE_ENABLE_NONE },
        { .colorWriteMask = COLOR_WRITE_ENABLE_NONE },
        { .colorWriteMask = COLOR_WRITE_ENABLE_NONE },
        { .colorWriteMask = COLOR_WRITE_ENABLE_NONE },
        { .colorWriteMask = COLOR_WRITE_ENABLE_NONE },
        { .colorWriteMask = COLOR_WRITE_ENABLE_NONE },
    },
};

}  // namespace cave
