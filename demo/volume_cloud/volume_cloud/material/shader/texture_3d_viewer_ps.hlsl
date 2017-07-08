#include "../../shader_include/noise.hlsli"
#include "../../shader_include/build_in_placement_ui_inout.hlsli"

SamplerState SS : register(s0);
Texture3D tex : register(t0);
cbuffer lay : register(b0)
{
    float layer;
    float4 filter;
};

cbuffer random_data : register(b1)
{
    uint3 size;
    float3 wise_noise_point[100];
    float3 perlin_noise_factor[4];
}

float4 main(placemenet_ui_vs_output input) : SV_TARGET
{
    return tex.Sample(SS, float3(input.texcoord.xy, layer)) * filter;
}