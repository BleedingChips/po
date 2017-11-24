#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
RWTexture3D<float> outputTexture : register(u0);
StructuredBuffer<float3> rpf1 : register(t0);
StructuredBuffer<float3> rpf2 : register(t1);

cbuffer b0 : register(b0)
{
    uint3 size;
    uint3 Block;
    uint WorleyCount;
    float Length;
}

cbuffer b1 : register(b1)
{
    uint count1;
    uint style1;
    float4 p11;
    float3 p21;
}

cbuffer b2 : register(b2)
{
    uint count2;
    uint style2;
    float4 p12;
    float3 p22;
}

float RateFF(float t)
{
    return (t * t * t * (t * (t * 6 - 15) + 10));
}


float PerlinNoise(float3 UV, uint3 Block)
{
    uint3 StartIndex = floor(UV * (Block));
    float3 Rate = frac(UV * (Block));
    StartIndex = step(StartIndex + 1, Block) * StartIndex;
    float AllValue[8];
    for (uint count = 0; count < 8; ++count)
    {
        uint3 Dir = uint3(count / 4, (count / 2) % 2, count % 2);
        float3 Point = Rate - Dir;
        Dir = StartIndex + Dir;
        Dir = step(Dir + 1, Block) * Dir;
        float3 PointValue = rpf1[Dir.z * Block.y * Block.x + Dir.y * Block.x + Dir.x].xyz;
        float3 PointValue2 = PointValue * PointValue;
        float3 Normal = float3(sin(PointValue.x) * cos(PointValue.y), sin(PointValue.x) * sin(PointValue.y), cos(PointValue.x));
        AllValue[count] = dot(Normal, Point);
        //AllValue[count] = PointValue.x;
    }
    float XValue[4];
    for (count = 0; count < 4; ++count)
    {
        XValue[count] = lerp(AllValue[count * 2], AllValue[count * 2 + 1], RateFF(Rate.z));
    }
    float YValue[2];
    for (count = 0; count < 2; ++count)
    {
        YValue[count] = lerp(XValue[count * 2], XValue[count * 2 + 1], RateFF(Rate.y));
    }
    return (lerp(YValue[0], YValue[1], RateFF(Rate.x)) + 1.0) / 2.0;
    //return lerp(YValue[0], YValue[1], RateFF(Rate.x));

}


float TilessWorleyPoint3(float3 uv, float Length, uint total_count)
{
    float Dis = 1.0;

    for (uint count = 0; count < total_count; ++count)
    {
        float3 PointPoi = rpf2[count];
        float3 Dir = abs(uv - PointPoi);
        Dir = min(Dir, 1.0 - Dir);
        Dis = min(Dis, length(Dir) * Length);
    }
    return 1.0 - Dis;
}


[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

    if (dot(step(DTid.xyz + 1, size), 1.0) >= 2.5)
    {
        float3 Poi = DTid / float3(size - 1);

        float PerlinValue = PerlinNoise(Poi, Block);

        float WorleyValue =
    TilessWorleyPoint3(Poi, Length, WorleyCount) * 0.5
    + TilessWorleyPoint3(frac(Poi * 2.0), Length, WorleyCount) * 0.25
    + TilessWorleyPoint3(frac(Poi * 3.0), Length, WorleyCount) * 0.125;
        +TilessWorleyPoint3(frac(Poi * 4.0), Length, WorleyCount) * 0.125;

		outputTexture[DTid] = clamp(PerlinValue - WorleyValue, 0.0, 1.0);
    }
}