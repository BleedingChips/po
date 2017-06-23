#include "../../shader_include/noise.hlsli"

StructuredBuffer<float3> da : register(t0);
RWTexture3D<float2> outSurface : register(u0);

cbuffer texture_size : register(b0)
{
    uint3 size;
};

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 rate = DTid / float3(size.x, size.y, size.z);

    float2 perlin_noise = float2(noise(rate * 8.0, float3(6.233, 0.22, 0.34)), noise(rate * 8.0, float3(6.3, 2.22, 1.54))) / 2.0 +
    float2(noise(rate * 16.0, float3(6.233, 0.22, 0.34)), noise(rate * 16.0, float3(6.3, 2.22, 1.54))) / 4.0 +
    float2(noise(rate * 32.0, float3(6.233, 0.22, 0.34)), noise(rate * 32.0, float3(6.3, 2.22, 1.54))) / 8.0 +
    float2(noise(rate * 64.0, float3(6.233, 0.22, 0.34)), noise(rate * 64.0, float3(6.3, 2.22, 1.54))) / 16.0;

    rate = (rate - float3(0.5, 0.5, 0.5)) * 2.0;

    float2 center = 0.0;
    for (uint count = 0; count != 100; ++count)
    {
        float d = (1.0 - 2.5 * distance(rate, da[count]));
        float2 d2 = float2(d, d);
        center = max(center, clamp(d2 + perlin_noise * 1.0, float2(0.0, 0.0), float2(1.0, 1.0)));
    }
    outSurface[DTid] = center;
    
}