#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture2D linearize_z : register(t0);
SamplerState ss : register(s0);


Texture2D Edge : register(t1);
Texture2D Height : register(t2);
Texture2D DensityMap : register(t3);

SamplerState HeightTextureSampler : register(s1);

cbuffer b0 : register(b0)
{
    float Density;
    float4 Value;
    float2 Rate;
}

cbuffer b1 : register(b1)
{
    property_local_transfer mat;
}
cbuffer b2 : register(b2)
{
    property_viewport_transfer ps;
}

float Sample2D4ChannelSimulate3D1ChannelWrap(Texture2D Tex, SamplerState SS, float3 SampleLocaltion, uint4 Block)
{
    float3 WrapLocation = frac(SampleLocaltion);
    //WrapLocation.z = frac(WrapLocation.z);
    float3 IntLoacation = WrapLocation * uint3(Block.x, Block.y, Block.z * Block.w * 4);

    uint ZChannelCount = Block.z * Block.w;
    uint ZCount = ZChannelCount * 4;
    float ZChunk = WrapLocation.z * (ZCount - 1);
    uint ZChunkLast = floor(ZChunk);
    uint ZChunkMax = ZChunkLast + 1;
    ZChunkMax = step(ZChunkMax + 1, ZCount) * ZChunkMax;
    float rate = ZChunk - ZChunkLast;
    uint2 LastChunkXY = uint2(ZChunkLast % Block.z, (ZChunkLast % ZChannelCount) / Block.z);
    uint2 MaxChunkXY = uint2(ZChunkMax % Block.z, (ZChunkMax % ZChannelCount) / Block.z);

    float2 FinalLocationLast = (WrapLocation.xy * 0.998 + 0.001 + LastChunkXY) / float2(Block.zw);
    float2 FinalLocationMax = (WrapLocation.xy * 0.998 + 0.001 + MaxChunkXY) / float2(Block.zw);

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
}



float FinalDensity(
Texture2D Denisty_T,
SamplerState Denisty_TSampler,
float4 SimulateSize_F4,
Texture2D Height_T,
SamplerState Height_TSampler,
float4 HeightChannelFactor_F4,
Texture2D Edge_T,
SamplerState Edge_TSampler,
float4 EdgeChannelFactor_F4,
float3 LocalStartPosition_F3,
float3 LocalEndPosition_F3,
float3 CubeSize_F3,
float Denisty_F,
float3 Mulity_F3,
float3 Shift_F3,
float DenistyCut_F,
float RayCount_F
)
{
    uint RayCount = RayCount_F;
    float3 Ray = (LocalEndPosition_F3 - LocalStartPosition_F3) / (2.0 * CubeSize_F3);
    float RayLength = length(Ray);
    float RayStepLength = RayLength / RayCount;
    float3 RayStep = normalize(Ray);
    float3 SampleStartPoint = (LocalStartPosition_F3 + CubeSize_F3) / CubeSize_F3 / 2.0;
    float FinalDensity = 0.0;
    
    for (uint count = 0; count < RayCount; ++count)
    {
        float3 SamplePoint = SampleStartPoint + RayStep * count * RayStepLength;
        float HeightValue = dot(Texture2DSample(Height_T, Height_TSampler, float2(0.5, SamplePoint.z)), HeightChannelFactor_F4);
        float EdgeValue = dot(Texture2DSample(Edge_T, Edge_TSampler, SamplePoint.xy), EdgeChannelFactor_F4);
        float DensityValue = Sample2D4ChannelSimulate3D1ChannelWrap(Denisty_T, Denisty_TSampler, SamplePoint * Mulity_F3 + Shift_F3, uint4(SimulateSize_F4));
        FinalDensity += clamp(HeightValue * DensityValue * EdgeValue - DenistyCut_F, 0.0, 1.0) * Denisty_F * RayStepLength;
    }
    //float4 FinalDensity = Sample2D4ChannelSimulate3D1ChannelWrap(Denisty_T, Denisty_TSampler, SampleStartPoint * Mulity_F3 + Shift_F3, uint4(SimulateSize_F4));
    //FinalDensity.w *= Denisty_F;
    return 1.0 - exp(-FinalDensity);
}


void main(in standard_ps_input input, out standard_ps_output_transparent output)
{
    
    const float3 WidthHeightDepth = float3(50, 50, 50);

    float3 EndLocalPosition;
    float3 StartLocalPosition;

    CalculateFlipNormalCubeRayStartEndLocalPosition(
    StartLocalPosition,
    EndLocalPosition,
    WidthHeightDepth,
    linearize_z.Sample(ss, get_uv_screen(input.uv_screen, input.position_sv)).x,
    input.position_world.xyz,
    property_viewport_transfer_eye_world_position(ps),
    property_viewport_transfer_eye_world_direction(ps),
    input.position_view.z,
    mat.world_to_local,
    near_clip_plane(ps)
);

    float RayResult = FinalDensity(
    DensityMap,
    HeightTextureSampler,
    float4(256, 256, 8, 8),
    Height,
    HeightTextureSampler,
    float4(1.0, 0.0, 0.0, 0.0),
    Edge,
    HeightTextureSampler,
    float4(1.0, 0.0, 0.0, 0.0),
    StartLocalPosition,
    EndLocalPosition,
    WidthHeightDepth,
    Density,
    float3(1.0, 1.0, 1.0),
    ps.time / 10000.0 * float3(1.0, 0.0, 0.0),
    Value.x,
		100
);


    output.color = float4(1.0, 1.0, 1.0, RayResult);
}



