#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
RWTexture3D<float> OutputTexture3 : register(u0);
StructuredBuffer<float3> Point : register(t0);

cbuffer b0 : register(b0)
{
    uint3 output_size;
    float Length;
    uint count;
}

float TilessWorleyPoint3(float3 uv, float Length, uint start, uint total_count)
{
    float Dis = 1.0;

    for (uint count = 0; count < total_count; ++count)
    {
        float3 PointPoi = Point[start + count];
        float3 Dir = abs(uv - PointPoi);
        Dir = min(Dir, 1.0 - Dir);
        Dis = min(Dis, length(Dir) * Length);
    }
    return 1.0 - Dis;
}



[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 Poi = DTid / float3(output_size - 1);

    float PrlinNoise = clamp((GradientPerlinNoiseRandTiled(Poi, uint3(5, 5, 5)) + 1.0) / 2.0, 0.0, 1.0) * 0.7
    + clamp((GradientPerlinNoiseRandTiled(frac(Poi * 2.0), uint3(5, 5, 5)) + 1.0) / 2.0, 0.0, 1.0) * 0.3;

    float WorleyNoise = 0.0
    + WorleyNoiseTileInputPoint(Poi, Length, Point, count) * 0.5
    + WorleyNoiseTileInputPoint(frac(Poi * 2.0), Length, Point, count) * 0.25
    + WorleyNoiseTileInputPoint(frac(Poi * 3.0), Length, Point, count) * 0.125
    ;

    OutputTexture3[DTid] = WorleyNoise;
    //clamp(PrlinNoise - 0.725 + WorleyNoise, 0.0, 1.0);


    
    
    /*
    TilessWorley(Poi + 0.23423, 5, 1.0) * 0.5;
   0.0 + TilessWorley(Poi + 0.2323, 12, 1.0) * 0.5 * 0.5
    + TilessWorley(Poi + 0.012, 27, 1.0) * 0.5 * 0.5 * 0.5
    + TilessWorley(Poi + 0.245, 40, 1.0) * 0.5 * 0.5 * 0.5;
*/
}