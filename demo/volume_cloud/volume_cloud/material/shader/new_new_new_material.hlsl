#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture2D BaseShapeTexture : register(t0);
SamplerState BaseShapeTextureSamplerState : register(s0);
Texture2D BaseShapeTexture2 : register(t1);

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

float SampleBaseDensity(Texture2D Tex1, SamplerState Tex1ss, float3 Poi)
{
    float SampleValue = Texture2DSample(Tex1, Tex1ss, Poi.xy).x;
    return clamp(SampleValue - Poi.z, 0.0, 1.0);
}

float SampleDetailDensity(Texture2D Tex1, SamplerState Tex1ss, float3 Poi, Texture3D Tex2, SamplerState Tex2ss, float3 XYZRate)
{
    float SampleValue = SampleBaseDensity(Tex1, Tex1ss, Poi).x;
    float SampleValue2 = Tex2.Sample(Tex2ss, Poi * XYZRate).x;
    return SampleValue * (1.0 - SampleValue2);
}

float SampleTextureImplement(Texture2D Tex, SamplerState Texss, float HeightMulity, float CenterHeight, float3 Poi)
{
    float SampleValue = Texture2DSample(Tex, Texss, Poi.xy).x;
    float Height = 0.0;
    if (Poi.z >= CenterHeight)
    {
        Height = (Poi.z - CenterHeight) / (1.0 - CenterHeight);
    }
    else
    {
        Height = (CenterHeight - Poi.z) / (CenterHeight);
    }
    Poi = abs(Poi - 0.5);
    float Fatcor = 1.0;
    if(Poi.x > 0.5 || Poi.y > 0.5 || Poi.z > 0.5)
        Fatcor = 0.0;
    return clamp(SampleValue * HeightMulity - Height, 0.0, 1.0) * Fatcor;
}


float2 RayMatchingBuildInNoise_Inside(
Texture2D HeightValue_T,
SamplerState HeightValue_TSampler,
float3 LocalStartPosition_F3,
float3 LocalEndPosition_F3,
float3 CubeSize_F3,
float2 CubeXYRate_F2,
float HeightMuliply_F,
float Density_F,
float HeightCenter_F,
float3 Light_F3,
float LightRayLength_F,
float LightReflex_F
)
{
    uint RayCount = 64;
    float3 CubeXYZRate = float3(CubeXYRate_F2, 1.0);
    float3 RayStep = (LocalEndPosition_F3 - LocalStartPosition_F3) / (2.0 * CubeSize_F3) / RayCount;
    float3 SampleStartPoint = (LocalStartPosition_F3 + CubeSize_F3) / CubeSize_F3 / 2.0;
    float RayLength = distance(LocalStartPosition_F3, LocalEndPosition_F3) / RayCount;
    float FrontDensity = 0.0;
    float FinalDensity = 0.0;
    uint ReachCount = RayCount;


    uint LightCount = 10;
    float3 LightRayStep = normalize(Light_F3) * LightRayLength_F / LightCount;
    float Color = 0.0;

    for (uint count = 1; count <= RayCount; ++count)
    {
        float3 SamplePoint = SampleStartPoint + RayStep * count;
        float Density = SampleTextureImplement(HeightValue_T, HeightValue_TSampler, HeightMuliply_F, 0.3, SamplePoint);
        float LightDensity = 0.0;
        float3 LightStartPoint = SamplePoint;
        for (uint count2 = 1; count2 <= LightCount; ++count2)
        {
            float3 LightSamplePoint = LightStartPoint + count2 * LightRayStep;
            float LightCurrentDensity = SampleTextureImplement(HeightValue_T, HeightValue_TSampler, HeightMuliply_F, 0.3, LightSamplePoint);
            LightDensity += LightCurrentDensity;
        }
        FinalDensity += Density * Density_F * RayLength;
        float Light = exp(-LightDensity * Density_F * LightRayLength_F) * exp(-FinalDensity) * Value.y / 10;
        
        Color += Light;
    }
    
    return float2(Color, 1.0 - exp(-FinalDensity));

    /*
    const uint DetailRayPowerCount = 64;
    const uint DetailRayDensityCount = 10;
    const float3 DetailCubeMultiply = 1.0;
    const float3 DetailLightRay = float3(0.0, 0.5, 0.0);
    const float3 DetailLightRayStep = DetailLightRay / DetailRayDensityCount;

    const float3 StartSamplePoint = SampleStartPoint + ReachCount * RayStep;

    

    const float3 DetailRayStep = RayStep / DetailRayPowerCount;

    float LightColor = 0.0;
    float DetailDensity = 0.0;
    for (uint count2 = 1; count2 <= DetailRayPowerCount; ++count2)
    {
        float3 SamplePoint = StartSamplePoint + count2 * DetailRayStep;
        float SampleValue = SampleDetailDensity(BaseShapeTexture, BaseShapeTextureSamplerState, SamplePoint, BaseShapeTexture2, BaseShapeTextureSamplerState, DetailCubeMultiply);
        float LightDensity = 0.0;
        for (uint count3 = 1; count3 <= DetailRayDensityCount; ++count3)
        {
            float3 SampleLightPoint = SamplePoint + count3 * DetailLightRayStep;
            float SampleValue = SampleDetailDensity(BaseShapeTexture, BaseShapeTextureSamplerState, SampleLightPoint, BaseShapeTexture2, BaseShapeTextureSamplerState, DetailCubeMultiply);
            LightDensity += SampleValue;
        }
        LightColor += exp(-LightDensity * Density * length(DetailLightRay)) * Value.x * (1.0 - exp(-SampleValue * Value.y));
        SampleValue = SampleValue * Density * length(DetailRayStep);
        LightColor = LightColor * exp(-SampleValue);
        DetailDensity += SampleValue;
    }*/

    
    /*
    for (uint count2 = 1; count2 <= DetailRayPowerCount; ++count2)
    {
        for (uint count3 = 1; count3 <= DetailRayDensityCount; ++count3)
        {
            float3 CurrentSamplePoint = StartSamplePoint + count3 * LightRay;
            float SampleValue = BaseShapeTexture2.Sample(BaseShapeTextureSamplerState, CurrentSamplePoint);
            RayDensity += (1.0 - SampleValue);
        }
    }*/



    /*
    const uint DetailRayPowerPointCount = 1;
    const uint DetailRayDensityCount = 10;
    const float DetailCubeMultiply = 4.0;
    float LightColor = 0.0;
    if (TotalCount > ReachCount)
    {
        float3 StartSamplePoint = SampleStartPoint + ReachCount * RayStep + float3(Move * Time, 0.0);
        StartSamplePoint = frac(StartSamplePoint * DetailCubeMultiply);
        float3 LightRay = float3(0.0, 1.0, 0.0) / DetailRayDensityCount;
        for (uint count2 = 1; count2 <= DetailRayPowerPointCount; ++count2)
        {
            float RayDensity = 0.0;
            for (uint count3 = 1; count3 <= DetailRayDensityCount; ++count3)
            {
                float3 CurrentSamplePoint = StartSamplePoint + count3 * LightRay;
                float SampleValue = BaseShapeTexture2.Sample(BaseShapeTextureSamplerState, CurrentSamplePoint);
                RayDensity += (1.0 - SampleValue);
            }
            RayDensity = -RayDensity * Density / DetailCubeMultiply;
            LightColor = exp(RayDensity) * (1.0 - exp(RayDensity * 2.0)) * Value.y * (1.0 - exp(-TargeDensity));
        }
    }
    */


    
    /*
    float3 ColorPosition = (LocalStartPosition + CubeSize_F3) / CubeSize_F3 / 2.0 + RayCountMin * RayStep;
    ColorPosition = ColorPosition * CubeSize_F3.z / CubeSize_F3;
    ColorPosition.z /= 8.0;
    float Color = BaseShapeTexture2.Sample(BaseShapeTextureSamplerState, frac(ColorPosition * 4.0)).x;
*/





    return float2(1.0, 1.0 - exp(-FinalDensity));
    //return float2(ReachCount / float(TotalCount), 1.0);



        //return float2(RayCountMin / 32.0, 1.0);
}

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{

    const float3 WidthHeightDepth = float3(80, 80, 10);

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
    BaseShapeTexture,
    BaseShapeTextureSamplerState,
    StartLocalPosition,
    EndLocalPosition,
    WidthHeightDepth,
    float2(0.5, 0.5),
    1.0,
    Density,
    0.3,
    float3(0.5, 0.5, 0.5),
    0.7, 
    Value.y
    );
    float Color = RayResult.x + Value.x;
    1.0;
    //RayResult.x;

    output.color = float4(Color, Color, Color, RayResult.y);
    //RayResult.y);
}



