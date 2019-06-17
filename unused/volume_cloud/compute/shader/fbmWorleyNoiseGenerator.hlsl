#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
RWTexture3D<float> Output : register(u0);
cbuffer b0 : register(b0)
{
    float3 Block;
    float3 Shift;
    float Mulity;
    uint Octaves;
    float Frequency;
    float Lacunarity;
    float Gain;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint3 size;
    Output.GetDimensions(size.x, size.y, size.z);
    float3 Poi = float3(DTid) / (size - 1);
    Output[DTid] = fbm_WorleyNoiseRand(Octaves, Frequency, Lacunarity, Gain, (Poi * Block) + Shift, Mulity);
}