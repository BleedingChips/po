#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture2D BaseShapeTexture : register(t0);
SamplerState BaseShapeTextureSamplerState : register(s0);

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

Texture2D linearize_z : register(t2);
SamplerState ss : register(s2);

float VolumeSample2(Texture2D Tex1, SamplerState TexSample1, float Base1, uint4 Tex1Simulate, Texture2D Tex2, SamplerState TexSample2, float Base2, uint4 Tex2Simulate, float3 Poi, float3 CycleExtand)
{
    float Value1 = Sample2D4ChannelSimulate3D1Channel(Tex1, TexSample1, Poi, Tex1Simulate);
    Value1 = max(Value1 - Base1, 0.0);
    float Value2 = Sample2D4ChannelSimulate3D1Channel(Tex2, TexSample2, (Poi + CycleExtand.x) * CycleExtand.y + CycleExtand.z, Tex2Simulate);
    Value2 = max(Value2 - Base2, 0.0);
    return max(Value1 - Value2, 0.0);
}

float CalculateLength(float3 Poi, float AttenuationHeight, float Density)
{
    //return 1.0;
    float DensityFactor = exp(-(1.0 - Density));
    float3 EdgeDetect = abs(Poi);
    float3 EdgeStep = step(0.5, EdgeDetect);
    EdgeDetect = EdgeDetect * EdgeStep;
    float3 EdgeTraget = 0.5 * EdgeStep;
    float Value = distance(EdgeTraget, EdgeDetect);
    float Factor = clamp(Value / -0.45 + 1, 0.0, 1.0);
    //float FinalHeightFactor = clamp((Poi.z / -AttenuationHeight + 1.0), 0.0, 1.0);
    return min(min(Factor, 1.0), 1.0);
}


float2 RayMatchingBuildInNoise_Inside(
float3 WorldStartPosition,
float3 WorldEndPosition,
float3 LocalStartPosition,
float3 LocalEndPosition,
float3 CubeSize_F3,
float Density,
float Time,
float2 Move
)
{
    uint RayCount = 64;
    float3 RayStep = (LocalStartPosition - LocalEndPosition) / (2.0 * CubeSize_F3) / RayCount;
    float3 SamplePoint = (LocalEndPosition + CubeSize_F3) / CubeSize_F3 / 2.0;
    float SampleDensity = 0.0;
    float RayLength = distance(WorldEndPosition, WorldStartPosition) / 64;
    float Color = 0.0;
    for (uint count = 0; count < RayCount; ++count)
    {
        float SampleValue = BaseShapeTexture.Sample(BaseShapeTextureSamplerState, (SamplePoint.xy / 2.0 + Move * Time) * 0.99).x;
        float PointDensity = max(SampleValue - SamplePoint.z, 0.0);
        SampleDensity += PointDensity;
        float LightPower = exp(-PointDensity * PointDensity / 2.0 * Density) * Value.x / 100.0; ;
        Color = LightPower + Color * exp(-SampleDensity * RayLength * Density) * (1.0 - exp(-SampleDensity * RayLength * Density * 2.0));
        SamplePoint += RayStep;
    }

    return float2(clamp(Color + Value.y, 0.0, 1.0), 1.0 - exp(-SampleDensity * Density * distance(WorldEndPosition, WorldStartPosition)));
}

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{

    const float3 WidthHeightDepth = float3(40, 40, 10);

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

    // 在世界坐标系下的最小深度
    float FinalWorldDepth = min(ReverseRayLenght, PixelMinDepth);

    float3 StartWorldPosition = FinalWorldDepth * -normalize(EyeRay) + EndWorldPosition;
    float3 StartLocalPosition = FinalWorldDepth * -LocalEyeRayWithLengthInformation + EndLocalPosition;


    float2 RayResult = RayMatchingBuildInNoise_Inside(
    StartWorldPosition,
    EndWorldPosition,
    StartLocalPosition,
    EndLocalPosition,
    WidthHeightDepth,
    Density,
    ps.time / 10000.0,
    float2(0.1, 0.03)
    );
    float Color = RayResult.x;
    1.0;
    //RayResult.x;

    output.color = float4(Color, Color, Color, RayResult.y);
    //RayResult.y);
}



