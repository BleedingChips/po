#ifndef VOLUME_CLOUD_COMPUTE_PROPERTY_HLSLI
#define VOLUME_CLOUD_COMPUTE_PROPERTY_HLSLI
struct property_random_point_f
{
    uint count;
    uint style;
    float4 parameter_1;
    float4 parameter_2;
};

struct property_random_point_f3
{
    uint count;
    uint style;
    float4 parameter_1;
    float4 parameter_2;
};

struct property_custom_random_point_f3
{
    uint count;
    uint4 type1;
    float4 parameter1;
    uint4 type2;
    float4 parameter2;
    uint4 type3;
    float4 parameter3;
};

void Simulate3D1ChannelFor2D4Channel(in uint2 index_2d, in uint4 simulate_size, 
out uint3 index_3d_r, out uint3 index_3d_g, out uint3 index_3d_b, out uint3 index_3d_a)
{
    uint3 simulate_poi = uint3(
    index_2d.xy % (simulate_size.xy + 2),
		index_2d.x / (simulate_size.x + 2) + (index_2d.y / (simulate_size.y + 2)) * (simulate_size.z)
);
    if (simulate_poi.x == 0)
        simulate_poi.x = simulate_size.x - 1;
    else if (simulate_poi.x == simulate_size.x + 1)
        simulate_poi.x = 0;
    else
        simulate_poi.x -= 1;

    if (simulate_poi.y == 0)
        simulate_poi.y = simulate_size.y - 1;
    else if (simulate_poi.y == simulate_size.y + 1)
        simulate_poi.y = 0;
    else
        simulate_poi.y -= 1;




    index_3d_r = simulate_poi;
    index_3d_g = simulate_poi + uint3(0, 0, simulate_size.z * simulate_size.w);
    index_3d_b = simulate_poi + uint3(0, 0, simulate_size.z * simulate_size.w * 2);
    index_3d_a = simulate_poi + uint3(0, 0, simulate_size.z * simulate_size.w * 3);
}
#endif