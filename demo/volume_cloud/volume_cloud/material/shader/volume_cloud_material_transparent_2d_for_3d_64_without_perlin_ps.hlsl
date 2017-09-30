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

    // ����ǲ�͸���������ȣ���Ҫ������͸�������ڵ�ʱ�������
    float OpaqueDepth = 0.0;
    {
        //��ȡ��ĻUV
        float2 screen_uv = get_uv_screen(input.uv_screen, input.position_sv);
        OpaqueDepth = linearize_z.Sample(ss, screen_uv).x;
    }

    // �������ص������벻͸���������ȵ���Сֵ���������ߵĿ�ʼ�㡣
    float PixelMinDepth = min(OpaqueDepth, input.position_view.z);

    // �����ӽ���������
    float3 EyeRay = input.position_world.xyz - property_viewport_transfer_eye_world_position(ps);

    // ͨ����������ȱȼ���ʵ�����ߵ�ʵ�ʿ�ʼ�㡣
    float3 StartWorldPosition = EyeRay * (PixelMinDepth / input.position_view.z) + property_viewport_transfer_eye_world_position(ps);

    // �����������Ϣ�ľֲ������µĹ�������
    float3 LocalEyeRayWithLengthInformation = mul(mat.world_to_local, float4(normalize(EyeRay), 0.0)).xyz;

    // ������������������±任�ɾֲ�����ϵ�µĳ��ȱ�
    float RayLengthWorldToLocal = length(LocalEyeRayWithLengthInformation);

    // ���㿪ʼ��ľֲ�����
    float3 StartLocalPosition;
    {
        float4 StartWorldPosition4 = float4(StartWorldPosition, 1.0);
        StartWorldPosition4 = mul(mat.world_to_local, StartWorldPosition4);
        StartLocalPosition = StartWorldPosition4.xyz / StartWorldPosition4.w;
    }

    // ���㷴����ߵ�λ�ƣ�ʵ���Ͼ��Ǵ���ʼ�㣬ͨ����������ƶ���������߽��λ��
    float ReverseRayLenght = RayPatch(-LocalEyeRayWithLengthInformation, StartLocalPosition, WidthHeightDepth);

    // ����������ϵ�µ���С���
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



