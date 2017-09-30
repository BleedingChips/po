#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"

Texture2D BaseShapeTexture : register(t0);
SamplerState BaseShapeTextureSamplerState : register(s0);

struct PropertyType
{
    float3 Scale;
    float3 Transform;
    float4 Factor;
};

cbuffer b0 : register(b0)
{
    PropertyType BaseShapeProperty;
}

Texture2D MoveMaskTexture : register(t1);
SamplerState MoveMaskTextureSamplerState : register(s1);

cbuffer b1 : register(b1)
{
    PropertyType MoveMaskProperty;
}

Texture2D MaskTexture : register(t2);
SamplerState MaskTextureSamplerState : register(s2);

cbuffer b2 : register(b2)
{
    PropertyType MaskProperty;
}

cbuffer b0 : register(b3)
{
    property_rendder_2d_for_3d pp;
}
cbuffer b1 : register(b4)
{
    property_local_transfer mat;
}
cbuffer b2 : register(b5)
{
    property_viewport_transfer ps;
}

Texture2D linearize_z : register(t3);
SamplerState ss : register(s3);

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{

    const float3 WidthHeightDepth = float3(50, 50, 50);

    // 这个是不透明物体的深度，主要处理被不透明物体遮挡时候的问题
    float OpaqueDepth = 0.0;
    {
        //获取屏幕UV
        float2 screen_uv = get_uv_screen(input.uv_screen, input.position_sv);
        OpaqueDepth = linearize_z.Sample(ss, screen_uv).x;
    }

    // 计算像素点的深度与不透明物体的深度的最小值，计算射线的开始点。
    float PixelMinDepth = min(OpaqueDepth, input.position_view.z);

    // 计算视角射线向量
    float3 EyeRay = input.position_world.xyz - property_viewport_transfer_eye_world_position(ps);

    // 通过向量和深度比计算实际射线的实际开始点。
    float3 StartWorldPosition = EyeRay * (PixelMinDepth / input.position_view.z) + property_viewport_transfer_eye_world_position(ps);

    // 计算带长度信息的局部坐标下的光线向量
    float3 LocalEyeRayWithLengthInformation = mul(mat.world_to_local, float4(normalize(EyeRay), 0.0)).xyz;

    // 计算深度在世界坐标下变换成局部坐标系下的长度比
    float RayLengthWorldToLocal = length(LocalEyeRayWithLengthInformation);

    // 计算开始点的局部坐标
    float3 StartLocalPosition;
    {
        float4 StartWorldPosition4 = float4(StartWorldPosition, 1.0);
        StartWorldPosition4 = mul(mat.world_to_local, StartWorldPosition4);
        StartLocalPosition = StartWorldPosition4.xyz / StartWorldPosition4.w;
    }

    // 计算反向光线的位移，实际上就是从起始点，通过反向光线移动到立方体边界的位移
    float ReverseRayLenght = RayPatch(-LocalEyeRayWithLengthInformation, StartLocalPosition, WidthHeightDepth);

    // 在世界坐标系下的最小深度
    float FinalWorldDepth = min(ReverseRayLenght, PixelMinDepth);
    
    float3 UnitRayPath;
    {
        float3 LocalRayWidthDepth = FinalWorldDepth * LocalEyeRayWithLengthInformation;
        UnitRayPath = -LocalRayWidthDepth / (2.0 * WidthHeightDepth);
    }

    float3 UnitPoint = (StartLocalPosition + WidthHeightDepth) / (2.0 * WidthHeightDepth);
    
    float RayResult = RayMatchingWidthNoLighting(
    BaseShapeTexture,
    BaseShapeTextureSamplerState,
    float3(1.0, 1.0, 1.0),
    float3(0.0, 0.0, 0.0),
    float2(1.0, 0.0),
    float4(256, 256, 8, 8),

    MoveMaskTexture,
    MoveMaskTextureSamplerState,
    float3(1.0, 1.0, 1.0),
    float3(0.0, 0.0, 0.0),
    float2(0.0, 0.0),
    float4(256, 256, 8, 8),

    MaskTexture,
    MaskTextureSamplerState,
    float2(0.0, 1.0),
    float4(64, 64, 4, 4),

    UnitPoint,
    UnitRayPath,
    pp.density,
    ps.time * 0.0001,
    //0.0,
    float3(0.0, 0.0, 1.0)
    );


    output.color = float4(1.0, 1.0, 1.0, RayResult);
}



