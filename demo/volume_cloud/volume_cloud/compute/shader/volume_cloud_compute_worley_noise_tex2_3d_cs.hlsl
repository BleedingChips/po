#include "volume_cloud_compute_property.hlsli"

cbuffer a1 : register(b0)
{
    property_worley_noise_3d_point poi;
}

cbuffer a2 : register(b1)
{
    property_output_tex2 output_tex2;
}

RWTexture2D<float4> out_texture : register(u0);

/*
cbuffer generator_patameter : register(b0)
{
    uint4 size;
    float step;
    float3 worley_noise_poin[n_point];
};
*/

float3 count_position(uint4 sim, uint2 input)
{
    float3 tem = float3(
    input.x % sim.x,
    input.y % sim.y,
    input.x / sim.x + sim.y / sim.y * sim.z
);
    return tem / float3(sim.x - 1, sim.y - 1, sim.z * sim.w - 1);
}




void compress(inout float i, inout float i2)
{
    float min_m = min(i, i2);
    i2 = max(i, i2);
    i = min_m;
}

void push_stack(in float d, inout float4 last)
{
    compress(last.w, d);
    compress(last.z, last.w);
    compress(last.y, last.z);
    compress(last.x, last.y);
}

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 max_dis = float4(1.0, 1.0, 1.0, 1.0);
    uint count = 0;
    float3 pos = count_position(output_tex2.simulate_3d_form_2d, DTid.xy);
    for (count = 0; count < worley_noise_3d_point_count; ++count)
    {
        float dis = min(distance(pos, poi.poi[count]) * output_tex2.step, 1.0);
        push_stack(dis, max_dis);
    }
    out_texture[DTid.xy] =
    //float4(1.0, 1.0, 1.0, 1.0);
    max_dis;
}