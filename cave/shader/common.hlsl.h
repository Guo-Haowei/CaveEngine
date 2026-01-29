/// File: common.hlsl.h
#if defined(HLSL_LANG)
#define MUL(a, b) mul((a), (b))
#elif defined(GLSL_LANG)
#define MUL(a, b) ((a) * (b))
#endif

// reconstruct view position
// https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy
float3 NdcToViewPos(float2 uv, float depth) {
    float2 ndc = 2.0f * uv - 1.0f;
#if defined(GLSL_LANG)
    float4 screen_pos = float4(ndc.x, ndc.y, 2.0f * depth - 1.0f, 1.0f);
#else
    float4 screen_pos = float4(ndc.x, ndc.y, depth, 1.0f);
#endif
    float4 viewPosH = MUL(c_invCamProj, screen_pos);
    float3 viewPos = viewPosH.xyz / viewPosH.w;
    return viewPos;
}
