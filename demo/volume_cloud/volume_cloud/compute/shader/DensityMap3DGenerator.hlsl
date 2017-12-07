#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
#include "volume_cloud_compute_property.hlsli"

RWTexture3D<float> output : register(u0);
StructuredBuffer<float3> Point : register(t0);

cbuffer b0 : register(b0)
{
    uint3 Size;
    uint3 Block;
    float Length;
}

cbuffer b1 : register(b1)
{
    property_random_point_f3 prp;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 Poi = DTid / float3(Size - 1);

    float PrlinNoise = clamp((GradientPerlinNoiseRandTiled(Poi, Block) + 1.0) / 2.0, 0.0, 1.0) * 0.5
    + clamp((GradientPerlinNoiseRandTiled(frac(Poi * 2.0), Block) + 1.0) / 2.0, 0.0, 1.0) * 0.25
    + clamp((GradientPerlinNoiseRandTiled(frac(Poi * 4.0), Block) + 1.0) / 2.0, 0.0, 1.0) * 0.125;
    PrlinNoise /= 0.725;

    float WorlyNoise = WorleyNoiseTileInputPoint(Poi, Length, Point, prp.count) * 0.5
    + WorleyNoiseTileInputPoint(frac(Poi * 2.0), Length, Point, prp.count) * 0.25
    + WorleyNoiseTileInputPoint(frac(Poi * 4.0), Length, Point, prp.count) * 0.125;
    WorlyNoise /= 0.725;

    output[DTid] =
    clamp(PrlinNoise + WorlyNoise - 1.0, 0.0, 1.0);

}