#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture3D BaseShapeTexture : register(t0);
SamplerState BaseShapeTextureSamplerState : register(s0);

Texture3D BaseShapeTexture2 : register(t1);
SamplerState BaseShapeTextureSamplerState2 : register(s1);

Texture2D BaseShapeTexture22 : register(t3);
Texture2D BaseShapeTexture222 : register(t4);

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


float RayMatchingBuildInNoise_Inside(
Texture2D Tex1_T,
SamplerState Tex1_TSampler,
float Tex1Cut_F,
float4 Tex1SimulateSize_F4,
Texture2D Tex2_T,
SamplerState Tex2_TSampler,
float Tex2Cut_F,
float4 Tex2SimulateSize_F4,
float3 CycleExtend_F3,
float3 CubeSize_F3,
float3 CubeLocalPosition_F3,
float3 CubeRayPath_F3,
float Density_F,
float Scale_F,
float Time_F,
float3 MoveDire
//float3 Light
)
{
    Density_F = Density_F / 100;
    uint4 Tex1Simulate = Tex1SimulateSize_F4;
    uint4 Tex2Simplate = Tex2SimulateSize_F4;
    //Light = -Light;
    float3 Scale2 = float3(1.0, 1.0, 1.0) * Scale_F;
   // float HG = dot(normalize(Light), normalize(CubeRayPath_F3));
    float G = 0.7;
    float G2 = G * G;
    //HG = 0.07957747154594 * (1 - G2) / pow(1 + G2 - 2 * G * HG, 1.5);
    static const uint SampleCount = 100;
    static const uint LightCount = 8;
    static const float LightDistance = 0.25;
    static const float LightPatch = LightDistance / LightCount;
    //float3 LightStep = normalize(Light) * LightDistance / 4;
    float RayPathLength = length(CubeRayPath_F3);// * (rand(CubeRayPath_F3) * 0.05 + 1.0);
    float StepRayLength = RayPathLength / SampleCount;
    float3 UnitRayStep = //2.0 * normalize(CubeRayPath_F3) / SampleCount;
    CubeRayPath_F3 / SampleCount;

    float3 NormalizePoint = (CubeLocalPosition_F3 / CubeSize_F3);
    float3 NormalizeRayStep = (UnitRayStep / CubeSize_F3);// * (1.0 + rand(CubeLocalPosition_F3) * 0.05 / SampleCount);

    float ResultDensity = 0.0;
    float LastDensity = 0.0;
    float Color = 0.0;
    float3 SamplePoint = NormalizePoint;
    uint count = 0;
    for (count = 0; count < (SampleCount - 1); ++count)
    {
        float3 ShiftSamplePoint = SamplePoint + Time_F * MoveDire;
        //float FinalSampleValue = VolumeSample(BaseShapeTexture, BaseShapeTextureSamplerState, Value.w, BaseShapeTexture2, BaseShapeTextureSamplerState2, Value.w, ShiftSamplePoint * Scale2, 0.3, 0.75, 0.5);
        float FinalSampleValue = VolumeSample2(Tex1_T, Tex1_TSampler, Tex1Cut_F, Tex1Simulate, Tex2_T, Tex2_TSampler, Tex2Cut_F, Tex2Simplate, ShiftSamplePoint * Scale2, CycleExtend_F3);
        //float HeightGround = clamp(0.5 - SamplePoint.z, 0.0, 1.0);
        float HeightGround = CalculateLength(SamplePoint, 0.6, FinalSampleValue);
        float FinalDensity = FinalSampleValue * HeightGround;
        ResultDensity = ResultDensity + FinalDensity;
        (FinalDensity + LastDensity) / 2.0;
        LastDensity = FinalDensity;
        /*
        //if (count < 20)
        {
            float3 LightSamplePoint = SamplePoint;
            float LightDensity = 0.0;
            for (uint count2 = 1; count2 < (LightCount - 1); ++count2)
            {
            //float Density = VolumeSample(BaseShapeTexture, BaseShapeTextureSamplerState, Value.w, BaseShapeTexture2, BaseShapeTextureSamplerState2, Value.w, (LightSamplePoint + Time_F * MoveDire) * Scale2, 0.3, 0.75, 0.5);
                float Density = VolumeSample2(BaseShapeTexture22, BaseShapeTextureSamplerState, Tex1, BaseShapeTexture222, BaseShapeTextureSamplerState2, Tex2, (LightSamplePoint + Time_F * MoveDire) * Scale2, 0.7, 1.345, 0.6);
                float HeightGround = CalculateLength(LightSamplePoint, 0.6, Density); // * step(abs(LightSamplePoint.x), 1.0001) * step(abs(LightSamplePoint.y), 1.0001);
                LightDensity += Density * HeightGround;
                LightSamplePoint += LightStep * (1.0 + rand(LightSamplePoint) * 0.05);
            }


            float LightPower = exp(-LightDensity * Density_F * LightPatch) * (1.0 - exp(-LightDensity * Density_F * LightPatch * 2.0));// * HG;
            float ED = exp(-FinalDensity * Density_F * StepRayLength);
            float Ray2333 = 1.0 / max(FinalDensity * Density_F, 0.000001) * (1.0 - ED);
            float Ray33 = (1.0 - exp(-Density_F * FinalDensity * Value.z));
            Color = LightPower * Value.y * Ray33 * Ray2333 + Color * ED;
        }
        */
        
       // Color *= ED;

        //Color *= exp(-FinalDensity * Density_F * StepRayLength);
        //Color += (1.0 - LightPower) * Value.y * RayPathLength;
        SamplePoint = SamplePoint + NormalizeRayStep * (1.0 + rand(SamplePoint) * 0.05);
    }

    float DensityE = exp(-ResultDensity * Density_F * RayPathLength);
    return 1.0 - DensityE;
    //return float2(1.0, 1.0 - DensityE);
    //return float2(clamp(Color + Value.x, 0.0, 0.9), 1.0 - DensityE);
    //return float2(1.0, 0.0);
}

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{

    const float3 WidthHeightDepth = float3(40, 40, 10);

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
        float3 LocalRayWidthDepth = FinalWorldDepth * LocalEyeRayWithLengthInformation;
        UnitRayPath = -LocalRayWidthDepth / (2.0 * WidthHeightDepth);

    float3 UnitPoint = (StartLocalPosition + WidthHeightDepth) / (2.0 * WidthHeightDepth);
    
    float RayResult = RayMatchingBuildInNoise_Inside(
    BaseShapeTexture22,
    BaseShapeTextureSamplerState,
    0.4,
    float4(256, 256, 4, 4),
    BaseShapeTexture222,
    BaseShapeTextureSamplerState,
    0.2,
    float4(256, 256, 4, 4),
    float3(0.45, 0.3455, 0.3),
    WidthHeightDepth,
    StartLocalPosition,
    -LocalRayWidthDepth,
    Density,
    0.5,
    ps.time / 1000.0,
    //0.0,
    float3(0.1, 0.0, 0.0)
    //,float3(-1.0, -1.0, 0.0)
    );
    float Color = 1.0;
    //RayResult.x;

    output.color = float4(Color, Color, Color, RayResult);
    //RayResult.y);
}



