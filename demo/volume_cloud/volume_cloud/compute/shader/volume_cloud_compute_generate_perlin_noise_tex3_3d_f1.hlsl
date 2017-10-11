#include "volume_cloud_compute_property.hlsli"

cbuffer b0 : register(b0)
{
    property_random_point_f ppn;
}

cbuffer b1 : register(b1)
{
    uint3 size;
    uint4 sample_scale;
    float4 factor;
}

StructuredBuffer<float> noise_point : register(t0);
RWTexture3D<float> output_texture : register(u0);

float cal_rate(float t)
{
    return (t * t * t * (t * (t * 6 - 15) + 10));
}

float4 rate4(float4 t)
{
    return float4(cal_rate(t.x), cal_rate(t.y), cal_rate(t.z), cal_rate(t.w));
}

uint cal_offset(uint pre_offset, uint3 start_point, uint scale)
{
    uint scale_size = scale + 1;
    return pre_offset + start_point.x * scale_size * scale_size + start_point.y * scale_size + start_point.z;
}


float cal_perlin_noise(uint point_offset, uint3 poisition, uint scale)
{
    uint3 area = size / scale;
    uint3 start_poi = poisition / area;
    float3 rate = float3(poisition % area) / float3(area);

    const uint3 shift_array[8] =
    {
        uint3(0, 0, 0),
        uint3(1, 0, 0),
        uint3(0, 1, 0),
        uint3(1, 1, 0),
        uint3(0, 0, 1),
        uint3(1, 0, 1),
        uint3(0, 1, 1),
        uint3(1, 1, 1),
    };

    float Result[8];
    uint count = 0;
    for (count = 0; count < 8; ++count)
        Result[count] = noise_point[cal_offset(point_offset, start_poi + shift_array[count], scale)];
    float4 Result_X = float4(
    lerp(Result[0], Result[1], cal_rate(rate.x)),
	lerp(Result[2], Result[3], cal_rate(rate.x)),
	lerp(Result[4], Result[5], cal_rate(rate.x)),
	lerp(Result[6], Result[7], cal_rate(rate.x))
);
    float2 Result_Y = float2(
    lerp(Result_X.x, Result_X.y, cal_rate(rate.y)),
	lerp(Result_X.z, Result_X.w, cal_rate(rate.y))
);
    return lerp(Result_Y.x, Result_Y.y, cal_rate(rate.z));
}








/*
float cal_perlin_noise(uint point_offset, uint3 poisition, uint scale)
{
    uint3 area = size / scale;
    uint3 start_poi = poisition / area;
    float3 rate = float3(poisition % area) / float3(area);
    
    float4 Result_1 = float4(
    noise_point[cal_offset(point_offset, start_poi, scale)],
    noise_point[cal_offset(point_offset, start_poi + uint3(1, 0, 0), scale)],
    noise_point[cal_offset(point_offset, start_poi + uint3(0, 1, 0), scale)],
    noise_point[cal_offset(point_offset, start_poi + uint3(0, 0, 1), scale)]
);
    float4 Length_1 = float4(
    distance(rate, float3(0, 0, 0)),
    distance(rate, float3(1, 0, 0)),
    distance(rate, float3(0, 1, 0)),
    distance(rate, float3(0, 0, 1))
);
    float4 Result_2 = float4(
    noise_point[cal_offset(point_offset, start_poi + uint3(1, 1, 0), scale)],
    noise_point[cal_offset(point_offset, start_poi + uint3(0, 1, 1), scale)],
    noise_point[cal_offset(point_offset, start_poi + uint3(1, 0, 1), scale)],
    noise_point[cal_offset(point_offset, start_poi + uint3(1, 1, 1), scale)]
);
    float4 Length_2 = float4(
    distance(rate, float3(1, 1, 0)),
    distance(rate, float3(0, 1, 1)),
    distance(rate, float3(1, 0, 1)),
    distance(rate, float3(1, 1, 1))
);
    float4 Length_1_Factor = (max(1.0 - Length_1, 0.0));
    float4 Length_2_Factor = (max(1.0 - Length_2, 0.0));


    float Length = dot(Length_1_Factor, 1.0) + dot(Length_2_Factor, 1.0);
    float color = dot(Result_1, Length_1_Factor / Length) + dot(Result_2, Length_2_Factor / Length);
    return color;
}*/

uint count_all(uint i)
{
    return (i + 1) * (i + 1) * (i + 1);
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (ppn.count >= count_all(sample_scale.x) + count_all(sample_scale.y) + count_all(sample_scale.z) + count_all(sample_scale.y))
    {
        uint3 loc = DTid;
        output_texture[DTid] = dot(float4(
        cal_perlin_noise(0, DTid, sample_scale.x),
        cal_perlin_noise(count_all(sample_scale.x), DTid, sample_scale.y),
        cal_perlin_noise(count_all(sample_scale.x) + count_all(sample_scale.y), DTid, sample_scale.z),
        cal_perlin_noise(count_all(sample_scale.x) + count_all(sample_scale.y) + count_all(sample_scale.z), DTid, sample_scale.w)
), factor);
    }
}