/// File: sprite.vs.hlsl
#include "cbuffer.hlsl.h"

struct VS_OUTPUT_UV {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

static const float2 TOP_LEFT = float2(-0.5f, +0.5f);
static const float2 TOP_RIGHT = float2(0.5f, +0.5f);
static const float2 BOTTOM_LEFT = float2(-0.5f, -0.5f);
static const float2 BOTTOM_RIGHT = float2(0.5f, -0.5f);

VS_OUTPUT_UV main(uint vert_id : SV_VertexID) {
    float2 positions[6] = {
        TOP_LEFT,
        BOTTOM_RIGHT,
        TOP_RIGHT,

        TOP_LEFT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
    };

    float2 pos = positions[vert_id];
    float4 position = float4(pos, 0.0f, 1.0f);
    position = mul(c_worldMatrix, position);
    position = mul(c_viewMatrix, position);
    position = mul(c_projectionMatrix, position);

    VS_OUTPUT_UV output;
    output.position = position;
    float2 local_uv = pos + float2(0.5f, 0.5f);

    // apply sprite uv
    output.uv = c_uv_rect.xy + local_uv * (c_uv_rect.zw - c_uv_rect.xy);
    return output;
}
