#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture2D HeightTexture : register(t0);
SamplerState HeightTextureSampler : register(s0);


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

Texture2D linearize_z : register(t2);
SamplerState ss : register(s2);

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

float SampleTextureImplement(Texture2D Tex, SamplerState Texss, float HeightMulity, float CenterHeight, float3 Poi, float2 XYRate, float2 XYShift1, float2 XYShift2, float Rate, float4 ValueFactor)
{
    float2 PoiXY1 = Poi.xy * XYRate + XYShift1;
    float2 PoiXY2 = Poi.xy * XYRate + XYShift2;
    float SampleValue1 = dot(Texture2DSample(Tex, Texss, PoiXY1), ValueFactor);
    float SampleValue2 = dot(Texture2DSample(Tex, Texss, PoiXY2), ValueFactor);

    float FinalValue = lerp(SampleValue1, SampleValue2, Rate);
    
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
    return clamp(FinalValue * HeightMulity - Height, 0.0, 1.0) * Fatcor;
}


float RayMatchingBuildInNoise_Inside(
Texture2D HeightValue_T,
SamplerState HeightValue_TSampler,
float3 LocalStartPosition_F3,
float3 LocalEndPosition_F3,
float3 CubeSize_F3,
float RayStepCount_F,
float HeightMuliply_F,
float Density_F,
float HeightCenter_F,
float2 SpriteFigureXYCount_F2,
float SpriteRate_F,
float4 ValueFactor_F4,
float2 XYScale_F2
)
{
    uint2 SpriteFigureXYCount = floor(SpriteFigureXYCount_F2);
    uint SpriteTotal = SpriteFigureXYCount.x * SpriteFigureXYCount.y;
    uint SpriteStart = floor(SpriteRate_F * (SpriteTotal));
    uint SpriteEnd = SpriteStart + 1;
    if (SpriteEnd == SpriteTotal)
        SpriteEnd = 0;
    float SpriteRate = frac(SpriteRate_F * (SpriteTotal));
    float2 SpriteMulity = 1.0 / SpriteFigureXYCount_F2;
    float2 SpriteShift1 = float2(SpriteStart % SpriteFigureXYCount.x, SpriteStart / SpriteFigureXYCount.x) * SpriteMulity;
    float2 SpriteShift2 = float2(SpriteEnd % SpriteFigureXYCount.x, SpriteEnd / SpriteFigureXYCount.x) * SpriteMulity;

    uint RayCount = floor(RayStepCount_F);
    float3 Ray = (LocalEndPosition_F3 - LocalStartPosition_F3) / (2.0 * CubeSize_F3);
    float RayLength = length(Ray);
    float RayStepLength = RayLength / RayCount;
    float3 RayStep = normalize(Ray);
    float3 SampleStartPoint = (LocalStartPosition_F3 + CubeSize_F3) / CubeSize_F3 / 2.0;
    uint ReachCount = RayCount;
    float FinalDensity = 0.0;
    float2 SmapleMulity = SpriteMulity * XYScale_F2;
    float2 SampleShift1 = SpriteShift1 + SpriteMulity * (1.0 - XYScale_F2) * 0.5;
    float2 SampleShift2 = SpriteShift2 + SpriteMulity * (1.0 - XYScale_F2) * 0.5;

    for (uint count = 0; count < RayCount; ++count)
    {
        float3 SamplePoint = SampleStartPoint + RayStep * count * RayStepLength;// * (1.0 + frac(sin(dot(RayStep * count, float3(12.9898, 78.233, 42.1897))) * 43758.5453) * 0.01);;
        float Density = SampleTextureImplement(HeightValue_T, HeightValue_TSampler, HeightMuliply_F, HeightCenter_F, SamplePoint, SmapleMulity, SampleShift1, SampleShift2, SpriteRate, ValueFactor_F4);
        FinalDensity += Density * Density_F * RayStepLength;
    }
    return 1.0 - exp(-FinalDensity);

    //return float2(SpriteStart / 16.0, 1.0);






    /*
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
*/




    //return float2(1.0, 1.0 - exp(-FinalDensity));
}

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


    float RayResult = RayMatchingBuildInNoise_Inside(
    HeightTexture,
    HeightTextureSampler,
    StartLocalPosition,
    EndLocalPosition,
    WidthHeightDepth,
    100,
    1.0,
    pvdv.Density,
    0.3,
    float2(12.0, 7.0),
    frac(ps.time / 1000),
    float4(0.0, 0.0, 0.0, 1.0),
    float2(0.98, 0.98)
    );
    float Color = //RayResult.x;
    1.0;
    //RayResult.x;
    output.color = float4(Color, Color, Color, RayResult);
}



