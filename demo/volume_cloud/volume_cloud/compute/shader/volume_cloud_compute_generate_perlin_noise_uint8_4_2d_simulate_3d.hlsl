#include "volume_cloud_compute_property.hlsli"

cbuffer b0 : register(b0)
{
    property_random_point_f ppn;
}

cbuffer b1 : register(b1)
{
    property_output_tex2_2d_simulate_3d tex;
}

StructuredBuffer<float> noise_point : register(t0);
RWTexture2D<uint4> output_texture : register(u0);

float rate(float t)
{
    return (t * t * t * (t * (t * 6 - 15) + 10));
}

uint cal_offset(uint pre_offset, uint3 local, uint3 lay)
{
    return pre_offset + local.x * lay.y * lay.z + local.y * lay.z + local.z;
}

float cal_perlin_noise(uint point_offset, uint3 location, float3 layout, uint3 lay)
{
    float3 loc_rate = location / layout;
    uint3 floor_loc = floor(loc_rate);
    float3 rate_v = loc_rate - floor_loc;

    //return noise_point[cal_offset(point_offset, floor_loc + uint3(0, 0, 0), lay)];

    float t1 = lerp(
    noise_point[cal_offset(point_offset, floor_loc + uint3(0, 0, 0), lay)],
    noise_point[cal_offset(point_offset, floor_loc + uint3(1, 0, 0), lay)],
    rate(rate_v.x)
);
    float t2 = lerp(
    noise_point[cal_offset(point_offset, floor_loc + uint3(0, 1, 0), lay)],
    noise_point[cal_offset(point_offset, floor_loc + uint3(1, 1, 0), lay)],
    rate(rate_v.x)
);
    float t3 = lerp(t1, t2, rate(rate_v.y));

    t1 = lerp(
    noise_point[cal_offset(point_offset, floor_loc + uint3(0, 0, 1), lay)],
    noise_point[cal_offset(point_offset, floor_loc + uint3(1, 0, 1), lay)],
    rate(rate_v.x)
);
    t2 = lerp(
    noise_point[cal_offset(point_offset, floor_loc + uint3(0, 1, 1), lay)],
    noise_point[cal_offset(point_offset, floor_loc + uint3(1, 1, 1), lay)],
    rate(rate_v.x)
);
    float t4 = lerp(t1, t2, rate(rate_v.y));
    return lerp(t3, t4, rate(rate_v.z));
}

uint count_all(uint i)
{
    return (i + 1) * (i + 1) * (i + 1);
}

uint3 count_each(uint i)
{
    return uint3(i + 1, i + 1, i + 1);
}

static const uint layout1 = 5;
static const uint layout2 = layout1 * 2;
static const uint layout3 = layout2 * 2;
static const uint layout4 = layout3 * 2;

float cal_final(uint offset, uint3 loc, uint3 texture_size)
{
    return 0.0
        + cal_perlin_noise(offset, loc, texture_size / float(layout1), count_each(layout1)) * 0.5
        + cal_perlin_noise(count_all(layout1) + offset, loc, texture_size / float(layout2), count_each(layout2)) * 0.25
        + cal_perlin_noise(count_all(layout1) + count_all(layout2) + offset, loc, texture_size / float(layout3), count_each(layout3)) * 0.125
        + cal_perlin_noise(count_all(layout1) + count_all(layout2) + count_all(layout3) + offset, loc, texture_size / float(layout4), count_each(layout4)) * 0.125;
        ;
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint3 tex_size = uint3(tex.simulate_size.x, tex.simulate_size.y, tex.simulate_size.z * tex.simulate_size.w);
    if (ppn.count >= count_all(layout4) + count_all(layout2) + count_all(layout1) + count_all(layout3))
    {
        uint3 loc = uint3(
        DTid.x % tex.simulate_size.x,
        DTid.y % tex.simulate_size.y,
        DTid.y / tex.simulate_size.y * tex.simulate_size.z + DTid.x / tex.simulate_size.x
        );

        uint channel_size = count_all(layout1) + count_all(layout2) + count_all(layout3) + count_all(layout4);

        float4 result = float4(
        cal_final(0, loc, tex_size),
        cal_final(channel_size, loc, tex_size),
        cal_final(channel_size * 2, loc, tex_size),
        cal_final(channel_size * 3, loc, tex_size)
        );


        output_texture[DTid.xy] = uint4(result * 255);
    }
}