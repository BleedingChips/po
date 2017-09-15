#include "volume_cloud_compute_property.hlsli"

cbuffer b0 : register(b0)
{
    property_random_point_f ppn;
}

cbuffer b1 : register(b1)
{
    property_generate_worley_noise_float4_2d_simulate_3d tex;
}

StructuredBuffer<float> noise_point : register(t0);
RWTexture2D<float4> output_texture : register(u0);


void swap_if_less(inout float in1, inout float in2)
{
    float i1 = in1;
    float i2 = in2;
    uint rate = step(i1, i2);
    in1 = lerp(i2, i1, rate);
    in2 = lerp(i1, i2, rate);
}


void push(inout float input, inout float4 result)
{
    swap_if_less(result.w, input);
    swap_if_less(result.z, result.w);
    swap_if_less(result.y, result.z);
    swap_if_less(result.x, result.y);
}

float calculate_pre_mark(float4 mark, float4 reault)
{
    float4 temporary = mark * reault;
    return temporary.x + temporary.y + temporary.z + temporary.w;
}


[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if(DTid.x < tex.texture_size.x && DTid.y < tex.texture_size.y && ppn.count >= 400 * 3)
    {
        uint3 tex_size = uint3(tex.simulate_size.x, tex.simulate_size.y, tex.simulate_size.z * tex.simulate_size.w);
        uint3 loc = uint3(
        DTid.x % tex.simulate_size.x,
        DTid.y % tex.simulate_size.y,
        DTid.y / tex.simulate_size.y * tex.simulate_size.z + DTid.x / tex.simulate_size.x
        );

        float3 loc_f = loc / float3(tex_size - 1);

        float4 loca_array = float4(100.0, 100.0, 100.0, 100.0);

        for (uint count = 0; count < 400; ++count)
        {
            float3 p = float3(noise_point[count * 3], noise_point[count * 3 + 1], noise_point[count * 3 + 2]);
            float dis = distance(loc_f, p);
            push(dis, loca_array);
        }
        output_texture[DTid.xy] = clamp(loca_array * tex.radius, 0.0, 1.0);
    }
}