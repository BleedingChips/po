#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
#include "volume_cloud_material_property.hlsli"

cbuffer b0 : register(b0)
{
    float Density;
}
cbuffer b1 : register(b1)
{
    property_local_transfer mat;
}
cbuffer b2 : register(b2)
{
    property_viewport_transfer ps;
}

Texture2D linearize_z : register(t0);
SamplerState ss : register(s0);
float fbm_SimplexNoise(in float2 n, in float s, in uint octaves, in float frequency, in float lacunarity, in float gain);

float2 RayMatchingBuildInNoise(float3 UnitPoint_F3, float Gap_F, float3 UnitRayPath_F3, float Density_F, float Time_F, float3 UnitMove_F3)
{
    static const uint SampleCount = 64;
    float RayPathLength = length(UnitRayPath_F3);
    float3 UnitRayStep = UnitRayPath_F3 / SampleCount;

    float ResultDensity = 0.0;
    float LastDensity = 0.0;
    float3 SamplePoint = UnitPoint_F3;
    uint count = 0;
    for (count = 0; count < (SampleCount - 1); ++count)
    {

        float3 ShiftSamplePoint = SamplePoint + Time_F * UnitMove_F3;
        //float FinalDensity = fbm_worley_noise(ShiftSamplePoint * 10, Gap_F, 3, 1.0, 1.8, 0.5);
        float FinalDensity = fbm_SimplexNoise(ShiftSamplePoint, 3, 1.0, 1.8, 0.5);
        ResultDensity = //ResultDensity + FinalDensity;
        (FinalDensity + LastDensity) / 2.0;
        LastDensity = FinalDensity;
        SamplePoint = SamplePoint + UnitRayStep;
    }

    float DensityE = exp(-ResultDensity * Density_F * RayPathLength);
    return 1.0 - DensityE;
    return float2(1.0, 0.0);
}

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{

    //output.color = fbm_worley_noise(input.position_world * 100, 1.0, 4, 1.0, 1.8, 0.5);
    //return ;
    //float4(1.0, 1.0, 1.0, RayResult);

    const float3 WidthHeightDepth = float3(50, 50, 25);

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
    
    float RayResult = RayMatchingBuildInNoise(


    UnitPoint,
    1.0,
    UnitRayPath,
    Density,
    ps.time * 0.001,
    //0.0,
    float3(0.0, 0.0, 1.0)
    ).y;


    output.color = float4(1.0, 1.0, 1.0, RayResult);
}



