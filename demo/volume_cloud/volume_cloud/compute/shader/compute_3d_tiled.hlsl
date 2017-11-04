#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
RWTexture3D<float> OutputTexture3 : register(u0);
StructuredBuffer<float3> Point : register(t0);


float TilessWorleyPoint3(float3 uv, float Length, uint start, uint total_count)
{
    float Dis = 1.0;

    for (uint count = 0; count < total_count; ++count)
    {
        float3 PointPoi = Point[start + count];
        for (uint count2 = 0; count2 < 27; ++count2)
        {
            float3 Shift = float3((count2 / 9) % 3, (count2 / 3) % 3, count2 % 3) -1.0;
            float Distance = distance(uv, PointPoi + Shift) * Length;
            Dis = min(Distance, Dis);
        }
    }
    return 1.0 - Dis;
}



[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 Poi = DTid / float3(31, 31, 31);

    OutputTexture3[DTid] =
    
    TilessWorleyPoint3(Poi, 5.0, 0, 50) * 0.6
    + TilessWorleyPoint3(frac(Poi * 2.0), 5.0, 0, 50) * 0.4;
    /*
    TilessWorley(Poi + 0.23423, 5, 1.0) * 0.5;
   0.0 + TilessWorley(Poi + 0.2323, 12, 1.0) * 0.5 * 0.5
    + TilessWorley(Poi + 0.012, 27, 1.0) * 0.5 * 0.5 * 0.5
    + TilessWorley(Poi + 0.245, 40, 1.0) * 0.5 * 0.5 * 0.5;
*/
}