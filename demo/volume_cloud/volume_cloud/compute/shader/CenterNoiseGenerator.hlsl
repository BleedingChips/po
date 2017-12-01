#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
#include "volume_cloud_compute_property.hlsli"
RWTexture3D<float> OutputTexture : register(u0);
StructuredBuffer<float3> RandomBuffer : register(t0);

SamplerState ss : register(s0);
Texture2D XY : register(t1);
Texture2D YZ : register(t2);
Texture2D XZ : register(t3);

cbuffer b0 : register(b0)
{
    uint3 output_size;
    float Distance;
}

cbuffer b1 : register(b1)
{
    property_custom_random_point_f3 pcrp;
}

float fbm_XXX_noise(in float3 n, in uint octaves, in float frequency, in float lacunarity, in float gain)
{
    float amplitude = gain;
    float total = 0.0;
    for (uint i = 0; i < octaves; ++i)
    {
        total += WorleyNoiseInputPoint(n * frequency, Distance / frequency, RandomBuffer, pcrp.count) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return total;
}


[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 POi = DTid / float3(output_size - 1);
    float3 PrePOi = (POi - 0.5) * 2.0;
    float Value =
    
    XY.SampleLevel(ss, POi.xy, 0).x * YZ.SampleLevel(ss, POi.yz, 0).x 
    
    
    * clamp((fbm_XXX_noise(PrePOi, 2, 0.8, 1.75, 1.0) / 2.0 + abs(GradientPerlinNoiseRand(POi, uint3(10, 10, 10))) - 0.5), 0.0, 1.0);




    OutputTexture[DTid] = Value;
    
    
    
    
    //+ WorleyNoiseTileInputPoint(frac(POi * 1.25), Distance, RandomBuffer, pcrp.count) * dot(step(POi * 1.25), 1)
    //fbm_XXX_noise(POi, 4, 0.8, 1.7, 0.5);
    ;
}