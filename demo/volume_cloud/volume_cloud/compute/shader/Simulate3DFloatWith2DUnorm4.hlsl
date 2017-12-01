#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
#include "volume_cloud_compute_property.hlsli"
Texture3D<float4> input : register(t0);
RWTexture2D<uint4> output : register(u0);

cbuffer b0 : register(b0)
{
    uint4 SimulateSize;
    float4 Factor;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint3 SimulatePoi[4];
    Simulate3D1ChannelFor2D4Channel(DTid.xy, SimulateSize, SimulatePoi[0], SimulatePoi[1], SimulatePoi[2], SimulatePoi[3]);
    output[DTid.xy] =
    //uint4(255, 25, 255, 0);
    uint4(
    dot(input[SimulatePoi[0]], Factor) * 255,
    dot(input[SimulatePoi[1]], Factor) * 255,
    dot(input[SimulatePoi[2]], Factor) * 255,
    dot(input[SimulatePoi[3]], Factor) * 255
);
}