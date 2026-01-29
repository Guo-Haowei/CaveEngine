/// File: shared_diffuse_irradiance.h
#define SAMPLE_STEP 0.025f

#if defined(GLSL_LANG)
void main() {
    float3 N = normalize(out_var_POSITION);
#else
float4 main(VS_OUTPUT_POSITION input)
    : SV_TARGET {
    float3 N = normalize(input.world_position);
#endif
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = cross(up, N);
    up = cross(N, right);

    float3 irradiance = float3(0.0, 0.0, 0.0);
    float samples = 0.0;

    for (float phi = 0.0; phi < 2.0 * MY_PI; phi += SAMPLE_STEP) {
        for (float theta = 0.0; theta < 0.5 * MY_PI; theta += SAMPLE_STEP) {
            float xdir = sin(theta) * cos(phi);
            float ydir = sin(theta) * sin(phi);
            float zdir = cos(theta);
            float3 sample_dir = xdir * right + ydir * up + zdir * N;
#if defined(GLSL_LANG)
            irradiance += textureLod(t_Skybox, sample_dir, 0.0).rgb * cos(theta) * sin(theta);
#else
            irradiance += t_Skybox.SampleLevel(s_cubemapClampSampler, sample_dir, 0.0).rgb * cos(theta) * sin(theta);
#endif
            samples += 1.0;
        }
    }

    irradiance = MY_PI * irradiance * (1.0 / samples);
#if defined(GLSL_LANG)
    out_color = float4(irradiance, 1.0);
#else
    return float4(irradiance, 1.0);
#endif
}