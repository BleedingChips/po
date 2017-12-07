#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
#include "volume_cloud_compute_property.hlsli"
RWTexture3D<float> output : register(u0);
RWTexture3D<float4> buffer : register(u1);
Texture3D input : register(t0);

cbuffer b0 : register(b0)
{
    uint3 OutputSize;
    uint3 InputSize;
    float4 InputFactor;
    float MaxDistance;
    uint MaxCount;
    uint CallInstance;
    float EdgeValue;
    uint TotalLength;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float ScanDiance = clamp(MaxDistance, 0.0, 1.0);
    float3 Poi = DTid / float3(OutputSize - 1);
    float3 StartPoi = Poi - ScanDiance / 2.0 + 1.0;
    uint3 StartIndex = StartPoi * InputSize - 1;
    uint3 ScanBlock = InputSize * MaxDistance + 2;
    uint FinalCount = TotalLength;
    InputSize.x * InputSize.y * InputSize.z;
    float3 BiggerMark;
    float3 SmallerMark;
    if (CallInstance == 0)
    {
        BiggerMark = float3(1.0, 1.0, 1.0);
        SmallerMark = float3(1.0, 1.0, 1.0);
    }
    else
    {
        BiggerMark = buffer[DTid * uint3(2, 1, 1)].xyz;
        SmallerMark = buffer[DTid * uint3(2, 1, 1) + uint3(1, 0, 0)].xyz;
    }
        
    uint count = 0;

    for (; count < MaxCount && CallInstance * MaxCount + count < FinalCount; ++count)
    {
        uint CurrentScanIndex = CallInstance * MaxCount + count;
        uint3 CurrentScanPoi = uint3(CurrentScanIndex % ScanBlock.x, (CurrentScanIndex / ScanBlock.x) % ScanBlock.y, CurrentScanIndex / ScanBlock.y / ScanBlock.x);
        CurrentScanPoi = StartIndex + CurrentScanPoi;
        CurrentScanPoi = CurrentScanPoi % InputSize;
        float Value = dot(input[CurrentScanPoi], InputFactor);
        float3 ScanPoi = CurrentScanPoi / float3(InputSize - 1);
        float3 Dir = Poi - ScanPoi;
        Dir = min(abs(Dir), abs(1.0 - Dir));
        float3 Dis = float3(
        length(Dir),
        Dir.x + Dir.y + Dir.z,
        min(Dir.x, min(Dir.y, Dir.z))
);
        if (Value >= EdgeValue)
        {
            BiggerMark = min(BiggerMark, Dis);
        }
        else
        {
            SmallerMark = min(SmallerMark, Dis);
        }
    }
    if (CallInstance * MaxCount + count >= FinalCount)
    {
        float3 FinalValue = clamp(max(BiggerMark, SmallerMark) * (step(BiggerMark, SmallerMark) * 2.0 - 1.0) / MaxDistance + 0.5, 0.0, 1.0);
        output[DTid] = float4(FinalValue, 1.0);
        //
        //output[DTid] = float4(FinalValue, 1.0, 1.0, 1.0);
    }
    else
    {
        buffer[DTid * uint3(2, 1, 1)] = float4(BiggerMark, 1.0);
        buffer[DTid * uint3(2, 1, 1) + uint3(1, 0, 0)] = float4(SmallerMark, 1.0);
    }

}