#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture3D Original : register(t0);
Texture2D Simulate : register(t1);
Texture2D Demo : register(t2);
SamplerState ss : register(s0);
cbuffer b0 : register(b0)
{
    float4 Value;
}

/*
float Sample2D4ChannelSimulate3D1ChannelMirro(Texture2D Tex, SamplerState SS, float3 SampleLocaltion, uint4 Block)
{
    float3 WrapLocation = abs(1.0 - fmod(abs(SampleLocaltion + 1.0), 2.0));

    float3 IntLoacation = WrapLocation * uint3(Block.x, Block.y, Block.z * Block.w * 4);

    uint ZChannelCount = Block.z * Block.w;
    uint ZCount = ZChannelCount * 4;
    float ZChunk = WrapLocation.z * (ZCount);
    ZChunk -= 0.5;
    ZChunk = max(ZChunk, 0);
    uint ZChunkLast = floor(ZChunk);
    uint ZChunkMax = ZChunkLast + 1;
    ZChunkMax = min(ZChunkMax, ZCount - 1);
    step(ZChunkMax + 1, ZCount) * ZChunkMax;
    float rate = ZChunk - ZChunkLast;
    uint2 LastChunkXY = uint2(ZChunkLast % Block.z, (ZChunkLast % ZChannelCount) / Block.z);
    uint2 MaxChunkXY = uint2(ZChunkMax % Block.z, (ZChunkMax % ZChannelCount) / Block.z);

    float2 Mulity = Block.xy / float2(Block.xy + 2);
    float2 Shift = 1.0 / float2(Block.xy + 2);

    float2 FinalLocationLast = (WrapLocation.xy * Mulity + LastChunkXY + Shift) / float2(Block.zw);
    float2 FinalLocationMax = (WrapLocation.xy * Mulity + Shift + MaxChunkXY) / float2(Block.zw);

    static const float4 IndexFactor[4] =
    {
        float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0), float4(0, 0, 0, 1)
    };

    float4 S1 = Texture2DSample(Tex, SS, FinalLocationLast);
    float4 S2 = Texture2DSample(Tex, SS, FinalLocationMax);
    float4 F1 = IndexFactor[ZChunkLast / ZChannelCount];
    float4 F2 = IndexFactor[ZChunkMax / ZChannelCount];

    float V1 = dot(S1, F1);
    float V2 = dot(S2, F2);

    return lerp(V1, V2, rate);
}*/

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{
    float3 Poi = float3(input.uv, Value.x);

    float T2Value = Sample2D4ChannelSimulate3D1ChannelMirro(Simulate, ss, Poi, uint4(64, 64, 8, 2));
    float T3Value = Original.Sample(ss, Poi).x;

    float Res = abs(T3Value - T2Value) * Value.y;

    float4 Final = Simulate.Sample(ss, input.uv);

    float2 UV2 = input.uv * 2.0;

    output.color =
    
    //Demo.Sample(ss, UV2);
    //Final;
    float4(Res, Res, Res, 1.0f);
}