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

    // 世界坐标系下的路径与深度的比值
    float Result = dot(PixelDir, CameraWorldDir);

    const float3 WidthHeightDepth = float3(50, 50, 50);

    // 这个是不透明物体的深度，主要处理被不透明物体遮挡时候的问题
    float OpaqueDepth = linearize_z.Sample(ss, get_uv_screen(input.uv_screen, input.position_sv)).x;

    // 计算像素点的深度与不透明物体的深度的最小值，计算射线的开始点。
    float PixelMinDepth = min(OpaqueDepth, input.position_view.z);

    // 计算视角射线向量
    float3 EyeRay = input.position_world.xyz - property_viewport_transfer_eye_world_position(ps);

    // 通过向量和深度比计算实际采样射线的。
    float3 EndWorldPosition = EyeRay * (PixelMinDepth / input.position_view.z) + property_viewport_transfer_eye_world_position(ps);

    // 计算带长度信息的局部坐标下的光线向量
    float3 LocalEyeRayWithLengthInformation = mul(mat.world_to_local, float4(normalize(EyeRay), 0.0)).xyz;

    // 计算深度在世界坐标下变换成局部坐标系下的长度比
    float RayLengthWorldToLocal = length(LocalEyeRayWithLengthInformation);

    // 计算结束点的局部坐标
    float3 EndLocalPosition;
    {
        float4 EndWorldPosition4 = float4(EndWorldPosition, 1.0);
        EndWorldPosition4 = mul(mat.world_to_local, EndWorldPosition4);
        EndLocalPosition = EndWorldPosition4.xyz / EndWorldPosition4.w;
    }

    // 计算反向光线的位移，实际上就是从起始点，通过反向光线移动到立方体边界的位移
    float ReverseRayLenght = RayPatch(-LocalEyeRayWithLengthInformation, EndLocalPosition, WidthHeightDepth);

    // 减去近采样面的距离，然后计算在当前射线下的实际距离
    float MinWorldDepth = (PixelMinDepth - near_clip_plane(ps)) / Result;

    // 在世界坐标系下的最小深度
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