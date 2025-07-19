/// File: sprite.vs.hlsl
#include "cbuffer.hlsl.h"

struct VS_OUTPUT_UV {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_OUTPUT_UV main(uint vert_id : SV_VertexID) {
    float2 positions[6] = {
        float2(-1.0, 1.0),
        float2(-1.0, -1.0),
        float2(1.0, 1.0),

        float2(1.0, 1.0),
        float2(-1.0, -1.0),
        float2(1.0, -1.0),
    };

    float2 pos = positions[vert_id];
    float4 position = float4(pos, -1.0f, 1.0f);
    position = mul(c_worldMatrix, position);
    position = mul(c_viewMatrix, position);
    position = mul(c_projectionMatrix, position);

    VS_OUTPUT_UV output;
    output.position = position;
    output.uv = 0.5f * pos + float2(0.5f, 0.5f);
    return output;
}
