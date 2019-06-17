ConsumeStructuredBuffer<float3> input_point : register(u0);
AppendStructuredBuffer<float4> output_point : register(u1);
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
#include "volume_cloud_compute_property.hlsli"
Texture2D Edge : register(t0);
SamplerState ss : register(s0);

[numthreads(32, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 input = input_point.Consume();
    float3 pre_input = input - 0.5;
    float Rate = 0.8;
    float Rat2 = Rate * Rate;
    float Rat4 = 0.618 * 0.618;


    if (pre_input.x * pre_input.x + pre_input.y * pre_input.y / Rat2 + pre_input.z * pre_input.z / Rat4 < 0.35 * 0.35)
    {
        output_point.Append(float4(input.xyz, 0.0));
    }

    /*
    if (Edge.SampleLevel(ss, input.xy, 0).x > 0.9)
    {
        float PerlinNoise = fbm_ValuePerlinNoiseRand(5, 0.8, 1.7, 0.5, input.xy, uint2(5, 5));
        output_point.Append(float4(input.xy, PerlinNoise, input.z));
    }
*/
}