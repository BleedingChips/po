#ifndef VOLUME_CLOUD_COMPUTE_PROPERTY_HLSLI
#define VOLUME_CLOUD_COMPUTE_PROPERTY_HLSLI
static const uint worley_noise_3d_point_count = 300;
struct property_worley_noise_3d_point
{
    float3 poi[worley_noise_3d_point_count];
    float step;
};
struct property_output_tex2
{
    uint4 simulate_3d_form_2d;
    uint2 texture_size;
    float step;
};
struct property_perline_worley_noise_3d_point
{
    float3 seed1;
    float3 seed2;
    float3 seed3;
    float3 seed4;
    float3 poi1[100];
    float3 poi2[200];
    float3 poi3[400];
};
#endif