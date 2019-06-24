#ifndef BUILD_IN_PROPERTY_BUFFER_TYPE_INCLUDE_H
#define BUILD_IN_PROPERTY_BUFFER_TYPE_INCLUDE_H

struct property_screen_static
{
    float4x4 projection;
    float near_plane;
    float far_plane;
    float xy_rage;
    float2 viewports_left_top;
    float2 viewports_right_button;
    float2 viewports_near_far;
};

struct property_screen
{
    float4x4 view;
    float4x4 world_to_screen;
    float4x4 screen_to_local;
    float3 view_position;
    float time;
};

struct property_transfer
{
    float4x4 local_to_world;
    float4x4 world_to_local;
};
#endif