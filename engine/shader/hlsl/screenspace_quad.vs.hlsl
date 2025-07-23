/// File: screenspace_quad.vs.hlsl
#include "hlsl/input_output.hlsl"

static const float2 TOP_LEFT = float2(-1.0f, +1.0f);
static const float2 TOP_RIGHT = float2(1.0f, +1.0f);
static const float2 BOTTOM_LEFT = float2(-1.0f, -1.0f);
static const float2 BOTTOM_RIGHT = float2(1.0f, -1.0f);

VS_OUTPUT_UV main(uint vert_id : SV_VertexID) {
    float2 positions[6] = {
        TOP_LEFT,
        BOTTOM_RIGHT,
        TOP_RIGHT,

        TOP_LEFT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
    };

    VS_OUTPUT_UV output;
    float2 pos = positions[vert_id];

    output.position = float4(pos, 0.0f, 1.0f);
    output.uv = 0.5f * (pos + 1.0f);
    output.uv.y = 1.0f - output.uv.y;
    return output;
}
