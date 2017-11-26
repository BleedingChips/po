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

void Simulate3D1ChannelFor2D4Channel(in uint2 index_2d, in uint4 simulate_size, 
out uint3 index_3d_r, out uint3 index_3d_g, out uint3 index_3d_b, out uint3 index_3d_a)
{
    uint3 simulate_poi = uint3(
    index_2d.xy % simulate_size.xy,
		index_2d.x + index_2d.y * simulate_size.z
);
    index_3d_r = simulate_poi;
    index_3d_g = simulate_poi + uint3(0, 0, simulate_size.z * simulate_size.w);
    index_3d_b = simulate_poi + uint3(0, 0, simulate_size.z * simulate_size.w * 2);
    index_3d_a = simulate_poi + uint3(0, 0, simulate_size.z * simulate_size.w * 3);
}
#endif