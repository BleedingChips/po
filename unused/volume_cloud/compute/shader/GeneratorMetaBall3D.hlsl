#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
#include "volume_cloud_compute_property.hlsli"
Texture2D<float> Area : register(t0);
RWTexture3D<float> output : register(u0);
SamplerState ss : register(s0);
StructuredBuffer<float3> original_point : register(t1);

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

    float3 OutputSize;
    output.GetDimensions(OutputSize.x, OutputSize.y, OutputSize.z);
    float Max = max(max(OutputSize.x, OutputSize.y), OutputSize.z);
    float3 Pos = DTid / (Max - 1);

    uint PointCount;
    uint Stri;
    original_point.GetDimensions(PointCount, Stri);


    float Len = 1.0;
    for (uint count = 0; count < PointCount; ++count)
    {
        float3 pp = original_point[count];
        float SampleValue = Area.SampleLevel(ss, pp.xy, 0).x;
        if (SampleValue > 0.1)
        {
            float Distance = length(Pos - pp);
            Len = min(Len, Distance);
        }
    }
    output[DTid] = Len;






    /*
    float3 size;
    output.GetDimensions(size.x, size.y, size.z);
    float3 Pos = DTid / (size - 1);
    //Pos.y = Pos.y * 0.5 + 0.25;

    float3 size2;
    output2.GetDimensions(size2.x, size2.y, size2.z);
    float3 Pos2 = DTid / (size2 - 1);
    //Pos2.y = Pos2.y * 0.5 + 0.25;

    uint psize;
    uint stri;
    original_point.GetDimensions(psize, stri);

    float Rate = 0.8;
    float Rat2 = Rate * Rate;
    float Rat4 = 0.618 * 0.618;

    float Len = 1.0;
    for (uint count = 0; count < psize; ++count)
    {
        float3 pp = original_point[count];
        float3 pre_input = original_point[count] - 0.5;
        if (pre_input.x * pre_input.x + pre_input.y * pre_input.y / Rat2 + pre_input.z * pre_input.z / Rat4 < 0.35 * 0.35)
        {
            float Distance = length(Pos - pp);
            Len = min(Len, Distance);
        }
    }
    

    output[DTid] = Len;


    
    float Worley =  //WorleyNoiseRand(Pos * 10, 1.0);
    fbm_WorleyNoiseRand(5, 0.8, 1.7, 0.5, Pos2 * 5.0, 1.0);

    output2[DTid] = Worley;

    */





    //output2[DTid] = Worley;
    
    //float Clole = fbm_
    //float Color = clamp((Depth * 0.02), 0.0, 1.0);
    /*
    if (InsideDistance > OutsideDistance)
        output[DTid] = OutsideDistance;
    else
        output[DTid] = -InsideDistance;
*/
}