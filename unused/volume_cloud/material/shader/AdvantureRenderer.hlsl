#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture2D linearize_z : register(t0);
SamplerState ss : register(s0);

Texture2D DensityMap : register(t3);
SamplerState HeightTextureSampler : register(s1);

Texture2D DetailMap : register(t4);
SamplerState DetailMapSampler : register(s2);


cbuffer b0 : register(b0)
{
    property_volumecloud_debug_value pvdv;
}

cbuffer b1 : register(b1)
{
    property_local_transfer mat;
}
cbuffer b2 : register(b2)
{
    property_viewport_transfer ps;
}

cbuffer b3 : register(b3)
{
    uint4 DensitySimulateSize;
    uint4 DetailSimulateSize;
}


float SampleDistance(Texture2D Density_T, SamplerState DensityMap_TSampler, uint4 DensitySimulateSize, Texture2D Detail_T, SamplerState Detail_TSampler, uint4 DetailSimulateSize, float3 Poi, float3 Scale, float3 Shift, float Mulity)
{
    float SampleValue =
    //max(Poi.z - fbm_ValuePerlinNoiseRand(4, 2.0, 1.8, 0.5, Poi.xy + ps.time / 10000.0 * float2(1.0, 0.0), uint2(10.0, 10.0)), 0.0);


    Sample2D4ChannelSimulate3D1ChannelClampLevel(Density_T, DensityMap_TSampler, Poi, DensitySimulateSize, 0)
    - Sample2D4ChannelSimulate3D1ChannelMirroLevel(Detail_T, Detail_TSampler, (Poi * Scale + Shift), DetailSimulateSize, 0) * Mulity;
    ;
    return //SampleValue;
    clamp(SampleValue, 0.0, 1.0);
}


float2 FinalDensity(
Texture2D BaseShape_T,
SamplerState BaseShape_TSampler,
float4 BaseShapeSimulateSize_F4,
Texture2D Detail_T,
SamplerState Detail_TSampler,
float4 DetailSimulateSize_F4,
float DetailScale_F,
float3 DetailShift_F3,
float DetailMulity_F,
float3 LocalStartPosition_F3,
float3 LocalEndPosition_F3,
float3 CubeSize_F3,
float RayCount_F,
float LightRayCount_F,
float TargetDistance_F,
float DensityMulity_F,
float3 SunLight_F3,
float Reflection_F
)
{

    float3 BaseShapeSize = float3(BaseShapeSimulateSize_F4.x, BaseShapeSimulateSize_F4.y, BaseShapeSimulateSize_F4.z * BaseShapeSimulateSize_F4.w * 4.0);
    BaseShapeSize /= max(max(BaseShapeSize.x, BaseShapeSize.y), BaseShapeSize.z);

    float3 DetailSize = float3(DetailSimulateSize_F4.x, DetailSimulateSize_F4.y, DetailSimulateSize_F4.z * DetailSimulateSize_F4.w * 4.0);
    DetailSize /= max(max(DetailSize.x, DetailSize.y), DetailSize.z);

    float3 DetailScale = BaseShapeSize / DetailSize * DetailScale_F;

    float3 SunLight = normalize(SunLight_F3);
    float3 StartPosition = (LocalStartPosition_F3 + CubeSize_F3) / CubeSize_F3 / 2.0;
    float3 EndPosition = (LocalEndPosition_F3 + CubeSize_F3) / CubeSize_F3 / 2.0;

    float3 Ray = EndPosition - StartPosition;
    float RayLength = length(Ray);
    float3 RayStep = Ray / RayLength;

    float3 RealStartPosition = StartPosition * BaseShapeSize;
    float3 RealEndPosition = EndPosition * BaseShapeSize;
    float3 RealRay = RealEndPosition - RealStartPosition;
    float RealRayLength = length(RealRay);

    float LengthMulity = RayLength / RealRayLength;

    uint TotalRayStep = RayCount_F;
    uint TotalLightCount = LightRayCount_F;
    float RayStepLength = RayLength / TotalRayStep;

    float Depth = 0.0;

    float CloudeInsideDelpth = 0.0;

    float TotalColor = 0.0;
    DensityMulity_F = DensityMulity_F / 100;
    DetailMulity_F *= TargetDistance_F;

    for (uint count = 0; count < TotalRayStep; ++count)
    {
        if (Depth < RealRayLength)
        {
            float3 CurrentSamplePoint = StartPosition + Depth * LengthMulity * RayStep;
            float DensityValue = SampleDistance(BaseShape_T, BaseShape_TSampler, BaseShapeSimulateSize_F4, Detail_T, Detail_TSampler, DetailSimulateSize_F4, CurrentSamplePoint, DetailScale, DetailShift_F3, DetailMulity_F);
            float Dep = abs(DensityValue - TargetDistance_F);
            float ShiftDep = max(Dep, 0.01);
            if (DensityValue < TargetDistance_F)
            {
                if (TotalColor <= 1.0)
                {
                    float LightShift = max(Dep, 0.02);
                    float LightLength = 0.0;
                    for (uint count2 = 0; count2 < TotalLightCount; ++count2)
                    {
                        float3 LightSamplePoint = CurrentSamplePoint + LightShift * SunLight;
                        float DenistyValue = SampleDistance(BaseShape_T, BaseShape_TSampler, BaseShapeSimulateSize_F4, Detail_T, Detail_TSampler, DetailSimulateSize_F4, LightSamplePoint, DetailScale, DetailShift_F3, DetailMulity_F);
                        LightLength += max(TargetDistance_F - DenistyValue, 0.0);
                        float TemShift = abs(DenistyValue - TargetDistance_F);
                        LightShift += max(TemShift, 0.01 * (1.0 + 0.01 * rand(LightSamplePoint)));
                    }
                    float Light = exp(-LightLength * DensityMulity_F) * exp(-(CloudeInsideDelpth) * DensityMulity_F) * Reflection_F * max(TargetDistance_F - DensityValue, 0.0);
                    TotalColor += Light;
                }
                if (Depth + ShiftDep > RealRayLength)
                {
                    CloudeInsideDelpth += max((RealRayLength - Depth), 0.0);
                }
                else
                    CloudeInsideDelpth += Dep;
            }
            Depth += ShiftDep;
        }
    }
    float Result = exp(-CloudeInsideDelpth * DensityMulity_F);
    return float2(TotalColor, 1.0 - Result);
}


void main(in standard_ps_input input, out standard_ps_output_transparent output)
{
    
    float3 WidthHeightDepth = pvdv.XYZSizeOfCube;

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

    float2 RayResult = FinalDensity(
    DensityMap,
    HeightTextureSampler,
    DensitySimulateSize,
    DetailMap,
    DetailMapSampler,
    DetailSimulateSize,
    0.5,
    ps.time / 10000.0 * float3(1.0, 0.0, 0.0),
    pvdv.InputValue.w,
    StartLocalPosition,
    EndLocalPosition,
    WidthHeightDepth,
    64,
    10,
    pvdv.InputValue.y,
    pvdv.InputValue.x,
    float3(1.0, 0.0, 1.0),
    pvdv.InputValue.z
);

    output.color = float4(lerp(float3(0.5, 0.4, 0.8), float3(1.0, 1.0, 1.0), clamp(RayResult.x * 10.0, 0.0, 1.0)), RayResult.y);
}
