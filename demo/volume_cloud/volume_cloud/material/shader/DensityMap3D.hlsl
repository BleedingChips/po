#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture2D linearize_z : register(t0);
SamplerState ss : register(s0);

Texture3D DensityMap : register(t3);
SamplerState HeightTextureSampler : register(s1);

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

float FinalDensity(
Texture3D Denisty_T,
SamplerState Denisty_TSampler,
float3 LocalStartPosition_F3,
float3 LocalEndPosition_F3,
float3 CubeSize_F3,
float Denisty_F,
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
        float DensityValue = Denisty_T.Sample(Denisty_TSampler, SamplePoint).x;
        FinalDensity += DensityValue * Denisty_F * RayStepLength;
    }
    //float4 FinalDensity = Sample2D4ChannelSimulate3D1ChannelWrap(Denisty_T, Denisty_TSampler, SampleStartPoint * Mulity_F3 + Shift_F3, uint4(SimulateSize_F4));
    //FinalDensity.w *= Denisty_F;
    return 1.0 - exp(-FinalDensity);
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

    float RayResult = FinalDensity(
    DensityMap,
    HeightTextureSampler,
    StartLocalPosition,
    EndLocalPosition,
    WidthHeightDepth,
    pvdv.Density,
		100
);


    output.color = float4(1.0, 1.0, 1.0, RayResult);
}
