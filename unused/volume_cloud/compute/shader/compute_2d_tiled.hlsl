#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
RWTexture2D<float> OutputTexture : register(u0);
RWTexture2D<float2> OutputTexture2 : register(u1);
RWTexture3D<float> OutputTexture3 : register(u2);
StructuredBuffer<float> Point : register(t0);
StructuredBuffer<float3> Point2 : register(t1);

float TilessWorley(float2 uv, uint s, float Length)
{
    float dis = 2.0;
    for (uint count = 0; count < 9; ++count)
    {
        float2 dir = float2((count / 3) % 3, count % 3) - 1.0;
        float2 p = floor(uv * s) + dir;
        p = lerp(s - 1, p, step(0.0, p));
        p = lerp(0.0, p, step(p, s - 1));
        float2 rate = //rand(p);
        float2(rand(p), rand(p.x));
        //float2(rand(p.x), rand(p.xy));
        float2 pre_rate = rate + dir - frac(uv * s);
        float d = length(pre_rate) * Length;
        dis = min(dis, d);
    }
    return 1.0 - dis;
}

float TilessWorleyPoint2(float2 uv, float Length, uint start, uint total_count)
{
    float Dis = 1.0;

    for (uint count = 0; count < total_count; ++count)
    {
        float2 PointPoi = float2(Point[(start + count) * 2], Point[(start + count) * 2 + 1]);
        float2 PoiDir = abs(uv - PointPoi);
        PoiDir = min(PoiDir, abs(1.0 - PoiDir));
        float Distance = length(PoiDir) * Length;
        Dis = min(Distance, Dis);

        /*
        for (uint count2 = 0; count2 < 9; ++count2)
        {
            float2 Shift = float2(count2 / 3, count2 % 3) - 1.0;
            float Distance = distance(uv, PointPoi + Shift) * Length;
            Dis = min(Distance, Dis);
        }*/
    }
    return 1.0 - Dis;
}


float TilessWorleyPointDir(float2 uv, uint start, uint total_count)
{
    float2 MaxDir = float2(1.0, 1.0);
    float Dis = length(MaxDir);
    for (uint count = 0; count < total_count; ++count)
    {
        float2 PointPoi = float2(Point[(start + count) * 2], Point[(start + count) * 2 + 1]);
        for (uint count2 = 0; count2 < 9; ++count2)
        {
            float2 Shift = float2(count2 / 3, count2 % 3) - 1.0;
            float2 CurDir = PointPoi + Shift - uv;
            float Distance = distance(uv, PointPoi + Shift);
            MaxDir = lerp(MaxDir, CurDir, step(Distance, Dis));
            Dis = min(Distance, Dis);

        }
    }
    return Dis;
}


float TilessWorleyPoint3(float3 uv, float Length, uint start, uint total_count)
{
    float Dis = 1.0;

    for (uint count = 0; count < total_count; ++count)
    {
        float3 PointPoi = Point2[start + count];
        for (uint count2 = 0; count2 < 27; ++count2)
        {
            float3 Shift = float3((count2 / 9) % 3, (count2 / 3) % 3, count2 % 3) -1.0;
            float Distance = distance(uv, PointPoi + Shift) * Length;
            Dis = min(Distance, Dis);
        }
    }
    return 1.0 - Dis;
}

float TilessWorleyPoint4(float3 uv, float Length, uint start, uint total_count)
{
    float Dis = 1.0;

    for (uint count = 0; count < total_count; ++count)
    {
        float3 PointPoi = Point2[start + count];
        for (uint count2 = 0; count2 < 27; ++count2)
        {
            float3 Shift = float3((count2 / 9) % 3, (count2 / 3) % 3, count2 % 3) - 1.0;
            float Distance = Length - distance(uv, PointPoi + Shift);
            Dis = lerp(Dis, Distance, step(abs(Distance), abs(Dis)));
        }
    }
    return 1.0 - Dis;
}



[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 Poi = DTid / float3(255, 255, 255);


    float FinalValue = TilessWorleyPoint2(Poi.xy, 15, 0, 100) * 0.5
    + TilessWorleyPoint2(frac(Poi.xy * 2.0), 15, 0, 100) * 0.25
    + TilessWorleyPoint2(frac(Poi.xy * 4.0), 15, 0, 100) * 0.125;

    OutputTexture[DTid.xy] = FinalValue;
    
    
    
    
    
    //(TilessWorleyPointDir(Poi, 0, 200) + 1.0) / 2;
    
    //+ TilessWorleyPoint2(Poi, 7.0, 300, 100) * pow(0.5, 3)
    //+ TilessWorleyPoint2(Poi, 5.0, 400, 100) * pow(0.5, 4) 
    ;
    /*
    TilessWorley(Poi + 0.23423, 5, 1.0) * 0.5;
   0.0 + TilessWorley(Poi + 0.2323, 12, 1.0) * 0.5 * 0.5
    + TilessWorley(Poi + 0.012, 27, 1.0) * 0.5 * 0.5 * 0.5
    + TilessWorley(Poi + 0.245, 40, 1.0) * 0.5 * 0.5 * 0.5;
*/
}