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

struct property_local_transfer
{
    float4x4 local_to_world;
    float4x4 world_to_local;
};

struct property_viewport_transfer
{
    float4x4 world_to_eye;
    float4x4 eye_to_world;
    float4x4 world_to_camera;
    float4x4 camera_to_world;
    float near_surface;
    float far_surface;
    float view_near_surface;
    float view_far_surface;
    float time;
};

float3 property_viewport_transfer_eye_world_position(property_viewport_transfer pvt)
{
    return pvt.world_to_eye._14_24_34;
}

#endif