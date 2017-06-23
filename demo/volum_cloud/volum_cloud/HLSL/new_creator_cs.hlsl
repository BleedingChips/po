#include "noise.hlsli"


StructuredBuffer<float> da : register(t0);
RWTexture2D<float2> outSurface : register(u0);

cbuffer texture_size : register(b0)
{
    uint2 size;
};


[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint2 d = DTid.xy;
    float2 rate = float2(d.x,d.y) / float2(size.x, size.y);
    outSurface[d] = float2(1.0, 0.0);
}