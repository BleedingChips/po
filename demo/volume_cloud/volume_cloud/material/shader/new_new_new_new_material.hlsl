#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture2D linearize_z : register(t0);
SamplerState ss : register(s0);

Texture3D BaseShape : register(t1);
SamplerState BaseShapeSampler : register(s1);

cbuffer b0 : register(b0)
{
    float Density;
    float4 Value;
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
    float3 PixelWorldPosition = input.position_world.xyz;
    float3 CameraWorldPosition = property_viewport_transfer_eye_world_position(ps);
    float3 CameraWorldDir = property_viewport_transfer_eye_world_direction(ps);

    float3 PixelDir = normalize(PixelWorldPosition - CameraWorldPosition);

    // ��������ϵ�µ�·������ȵı�ֵ
    float Result = dot(PixelDir, CameraWorldDir);

    const float3 WidthHeightDepth = float3(50, 50, 50);

    // ����ǲ�͸���������ȣ���Ҫ������͸�������ڵ�ʱ�������
    float OpaqueDepth = linearize_z.Sample(ss, get_uv_screen(input.uv_screen, input.position_sv)).x;

    // �������ص������벻͸���������ȵ���Сֵ���������ߵĿ�ʼ�㡣
    float PixelMinDepth = min(OpaqueDepth, input.position_view.z);

    // �����ӽ���������
    float3 EyeRay = input.position_world.xyz - property_viewport_transfer_eye_world_position(ps);

    // ͨ����������ȱȼ���ʵ�ʲ������ߵġ�
    float3 EndWorldPosition = EyeRay * (PixelMinDepth / input.position_view.z) + property_viewport_transfer_eye_world_position(ps);

    // �����������Ϣ�ľֲ������µĹ�������
    float3 LocalEyeRayWithLengthInformation = mul(mat.world_to_local, float4(normalize(EyeRay), 0.0)).xyz;

    // ������������������±任�ɾֲ�����ϵ�µĳ��ȱ�
    float RayLengthWorldToLocal = length(LocalEyeRayWithLengthInformation);

    // ���������ľֲ�����
    float3 EndLocalPosition;
    {
        float4 EndWorldPosition4 = float4(EndWorldPosition, 1.0);
        EndWorldPosition4 = mul(mat.world_to_local, EndWorldPosition4);
        EndLocalPosition = EndWorldPosition4.xyz / EndWorldPosition4.w;
    }

    // ���㷴����ߵ�λ�ƣ�ʵ���Ͼ��Ǵ���ʼ�㣬ͨ����������ƶ���������߽��λ��
    float ReverseRayLenght = RayPatch(-LocalEyeRayWithLengthInformation, EndLocalPosition, WidthHeightDepth);

    // ��ȥ��������ľ��룬Ȼ������ڵ�ǰ�����µ�ʵ�ʾ���
    float MinWorldDepth = (PixelMinDepth - near_clip_plane(ps)) / Result;

    // ����������ϵ�µ���С���
    float FinalWorldDepth = min(ReverseRayLenght, MinWorldDepth);

    float3 StartWorldPosition = FinalWorldDepth * -normalize(EyeRay) + EndWorldPosition;
    float3 StartLocalPosition = FinalWorldDepth * -LocalEyeRayWithLengthInformation + EndLocalPosition;


    float2 RayResult = CalIMp(StartLocalPosition, EndLocalPosition, WidthHeightDepth, BaseShape, BaseShapeSampler, Density, Value, ps.time / 1000.0, float2(0.05, 0.0));


    float Color = RayResult.x;
    1.0;
    //RayResult.x;

    output.color = float4(Color, Color, Color, RayResult.y);

}

float2 CalIMp(float3 LocalStartPosition_F3, float3 LocalEndPosition_F3, float3 CubeSize_F3, Texture3D Tex_T, SamplerState Tex_TSampler, float Density, float4 Value, float Time_F, float2 Move_F2)
{
    uint RayCount = 32;
    float3 Ray = (LocalEndPosition_F3 - LocalStartPosition_F3) / (2.0 * CubeSize_F3);
    float3 RayStep = normalize(Ray);
    float3 SamplePoint = (LocalStartPosition_F3 + CubeSize_F3) / CubeSize_F3 / 2.0;
    float min_step = 0.001;
    float FinalDensity = 0.0;
    uint ReachCount = RayCount;
    for (uint count = 0; count <= RayCount; ++count)
    {
        
        float3 SampleValue = Tex_T.Sample(Tex_TSampler, SamplePoint * 0.5).xyz;
        float FinalValue = (SampleValue.x - 0.5) * 2.0 / 2.0;
        if (FinalValue > -min_step)
        {
            if (ReachCount >= count)
                ReachCount = count;
        }
        if (abs(FinalValue - Value.x) < -min_step)
            SamplePoint += min_step * 2 * RayStep;
        else
            SamplePoint += abs(FinalValue) * RayStep;
        if (dot(step(abs(SamplePoint - 0.5), 0.5001), 1.0) <= 2.5)
        {
            break;
        }
    }
    
    return float2(1.0, 1.0 - ReachCount / float(RayCount));
}