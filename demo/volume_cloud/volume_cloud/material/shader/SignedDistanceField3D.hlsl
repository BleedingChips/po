#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture2D linearize_z : register(t0);
SamplerState ss : register(s0);

Texture3D BaseShape : register(t1);
SamplerState BaseShapeSampler : register(s1);
Texture2D BaseShapeDif : register(t2);

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

float2 CalIMp(float3 StartLocalPosition_F3, float3 EndLocalPosition_F3, float3 CubeSize_F3, Texture3D Tex_T, SamplerState Tex_TSampler, float Density, float4 Value, float Time_F, float2 Move_F2);

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{
    const float3 WidthHeightDepth = pvdv.XYZSizeOfCube;

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


    float2 RayResult = CalIMp(
    StartLocalPosition, EndLocalPosition, 
    WidthHeightDepth, BaseShape, BaseShapeSampler, pvdv.Density, pvdv.InputValue, ps.time / 1000.0, float2(0.05, 0.0));


    float Color = RayResult.x;
    1.0;
    //RayResult.x;

    output.color = float4(Color, Color, Color, RayResult.y);

}

float2 CalIMp(float3 LocalStartPosition_F3, float3 LocalEndPosition_F3, float3 CubeSize_F3, Texture3D Tex_T, SamplerState Tex_TSampler, float Density, float4 Value, float Time_F, float2 Move_F2)
{
    uint RayCount = 64;
    float3 Ray = (LocalEndPosition_F3 - LocalStartPosition_F3) / (2.0 * CubeSize_F3);
    float3 RayStep = normalize(Ray);
    float3 SamplePoint = (LocalStartPosition_F3 + CubeSize_F3) / CubeSize_F3 / 2.0;
    float min_step = 0.001;
    float FinalDensity = 0.0;
    uint ReachCount = RayCount;


    SamplePoint += Value.x / 100.0 * Ray;

    /*
    float SampleValue =
        Sample2D4ChannelSimulate3D1ChannelWrap(BaseShapeDif, Tex_TSampler, SamplePoint, uint4(126, 126, 8, 4));
    float SampleValue2 =
        Tex_T.Sample(Tex_TSampler, SamplePoint).x;

    return float2(abs(SampleValue - SampleValue2) * 10.0, 1.0);
*/


    for (uint count = 0; count <= RayCount; ++count)
    {
        
        float SampleValue = 
        Sample2D4ChannelSimulate3D1ChannelWrap(BaseShapeDif, Tex_TSampler, SamplePoint + float3(Time_F * Move_F2, 0.0), uint4(126, 126, 8, 4));
        float SampleValue2 = 
        Tex_T.Sample(Tex_TSampler, SamplePoint).x;
        float FinalValue = (SampleValue.x - 0.5) * 0.25 * 2.0;
        if (FinalValue >= -min_step)
        {
            if (ReachCount >= count)
                ReachCount = count;
        }
        SamplePoint += abs(FinalValue) * RayStep;
        if (dot(step(abs(SamplePoint - 0.5), 0.5001), 1.0) <= 2.5)
        {
            break;
        }
    }
    
    return float2(1.0, 1.0 - ReachCount / float(RayCount));
}