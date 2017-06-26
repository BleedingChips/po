#include "../../shader_include/noise.hlsli"

StructuredBuffer<float3> da : register(t0);
RWTexture3D<float> outSurface : register(u0);

cbuffer texture_size : register(b0)
{
    uint3 size;
};

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 rate = DTid / float3(size.x, size.y, size.z);

    float2 perlin_noise = float2(noise(rate * 8.0, float3(6.233, 0.22, 0.34)), noise(rate * 16.0, float3(6.3, 2.22, 1.54))) / 2.0 +
    float2(noise(rate * 16.0, float3(6.233, 0.22, 0.34)), noise(rate * 32.0, float3(6.3, 2.22, 1.54))) / 4.0 +
    float2(noise(rate * 32.0, float3(6.233, 0.22, 0.34)), noise(rate * 64.0, float3(6.3, 2.22, 1.54))) / 8.0 +
    float2(noise(rate * 64.0, float3(6.233, 0.22, 0.34)), noise(rate * 128.0, float3(6.3, 2.22, 1.54))) / 8.0;

    float2 center = 0.0;
    for (uint count = 0; count != 100; ++count)
    {
        float d = (1.0 - 2.5 * distance(rate, da[count]));
        float2 d2 = float2(d, d);
        //center = max(center, clamp(d2 + (perlin_noise - 0.5) * 2.0, float2(0.0, 0.0), float2(1.0, 1.0)));
        float2 center2 = distance(rate, da[count]);
        //center = max(center, clamp(1.0 - center2, 0.0, 1.0));
        if (distance(rate, da[count]) < 0.05)
            center2 = 1.0;
        else
            center2 = 1 - (distance(rate, da[count]) - 0.05) * 4.0;
        center = max(center, center2);
    }
    //outSurface[DTid] = float2(r, 0.0);

    float r = DTid.z / float(size.z - 1);
    outSurface[DTid] = (center * perlin_noise * clamp(min(40 * r - 0.05, -20 * r + 18), 0.0, 1.0)).x;
    //outSurface[DTid] = center; //* perlin_noise * clamp(min(40 * r - 0.05, -20 * r + 18), 0.0, 1.0);
}