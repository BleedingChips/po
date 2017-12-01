#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
#include "volume_cloud_compute_property.hlsli"
RWTexture3D<float> output : register(u0);
RWTexture3D<float2> buffer : register(u1);
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
    float2 BackUp;
    if (CallInstance == 0)
        BackUp = float2(1.0, 1.0);
    else
        BackUp = buffer[DTid].xy;
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
        float Dis = length(Dir);
        if (Value >= EdgeValue)
        {
            BackUp.x = min(BackUp.x, Dis);
        }
        else
        {
            BackUp.y = min(BackUp.y, Dis);
        }
    }
    if (CallInstance * MaxCount + count >= FinalCount)
    {
        float FinalValue = clamp(max(BackUp.x, BackUp.y) * (step(BackUp.x, BackUp.y) * 2.0 - 1.0) + 0.5, 0.0, 1.0);
        output[DTid] = FinalValue;
        //
        //output[DTid] = float4(FinalValue, 1.0, 1.0, 1.0);
    }
    else
    {
        buffer[DTid] = float2(BackUp);
    }

}