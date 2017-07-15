#ifndef BUILD_IN_PROPERTY_BUFFER_TYPE_INCLUDE_H
#define BUILD_IN_PROPERTY_BUFFER_TYPE_INCLUDE_H
struct renderer_3d_buffer_t
{
    float4x4 world_to_screen;
    float4x4 screen_to_world;
    float rate;
    float time;
};

struct transfer_3d_static_buffer_t
{
    float4x4 local_to_world;
    float4x4 world_to_local;
};

struct transfer_2d_static_buffer_t
{
    bool adapt_screen;
    bool yes_true;
    float2 shift;
    float3 roll_and_center; //x -> roll angle and yz is center;
    float4 scale_and_center; // xy -> scale and zw is center;
};
#endif