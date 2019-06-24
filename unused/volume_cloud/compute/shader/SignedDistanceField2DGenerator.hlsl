#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
#include "volume_cloud_compute_property.hlsli"
RWTexture2D<uint4> OutputTexture : register(u0);
StructuredBuffer<float3> RandomPoint : register(t0);
cbuffer b0 : register(b0)
{
    uint2 OutputSize;
    uint4 SimulateSize;
    uint RandomPointCount;
    float DistanceMulity;
    float EdgeValue;
    float DFMaxDistance;
}

float CalCulateSDF(uint3 SimulatePoi)
{
    float3 Poi = SimulatePoi / (float3(SimulateSize.xy, SimulateSize.z * SimulateSize.w * 4) - 1.0);
    float WorleyValue = WorleyNoiseTileInputPoint(Poi, DistanceMulity, RandomPoint, RandomPointCount);
    return 0;
}





[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint3 SimulatePoi[4];
    Simulate3D1ChannelFor2D4ChannelWrap(DTid.xy, SimulateSize, SimulatePoi[0], SimulatePoi[1], SimulatePoi[2], SimulatePoi[3]);

}