#include "volume_cloud_compute_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
cbuffer b0 : register(b0)
{
    property_perline_worley_noise_3d_point pp;
};

cbuffer b1 : register(b1)
{
    property_output_tex2 pp2;
};

RWTexture2D<half4> output : register(u0);


float3 count_position(uint4 sim, uint2 input)
{
    float3 tem = float3(
    input.x % sim.x,
    input.y % sim.y,
    input.x / sim.x + input.y / sim.y * sim.z
);
    return tem / float3(sim.x - 1, sim.y - 1, sim.z * sim.w - 1);
}

float calculte_distance(float3 poi, float3 target, float target_dis)
{
    float d = distance(poi, target);
    return clamp((-5.0 / target_dis) * d + 6.0, 0.0, 1.0);
}


[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 pos = count_position(pp2.simulate_3d_form_2d, DTid.xy);
    float perlin_noise_v =
        perlin_noise(pos * 16.0, pp.seed1) / 2.0
    + perlin_noise(pos * 32.0, pp.seed2) / 4.0
    + perlin_noise(pos * 64.0, pp.seed3) / 8.0
    + perlin_noise(pos * 128.0, pp.seed4) / 8.0;

    uint count = 0;
    float dis1 = 0.0;
    for (count = 0; count < 100; ++count)
    {
        dis1 = max(dis1, calculte_distance(pos, pp.poi1[count], 0.1));
    }

    float dis2 = 0.0;
    for (count = 0; count < 200; ++count)
    {
        dis2 = max(dis2, calculte_distance(pos, pp.poi2[count], 0.05));
    }

    float dis3 = 0.0;
    for (count = 0; count < 200; ++count)
    {
        dis3 = max(dis3, calculte_distance(pos, pp.poi3[count], 0.025));
    }

    output[DTid.xy] = half4(clamp(float4(perlin_noise_v, dis1, dis2, dis3), 0.0, 1.0));
}