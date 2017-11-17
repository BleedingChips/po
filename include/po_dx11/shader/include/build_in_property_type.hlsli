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
    float4 near_far_viewnear_viewfar;
    float time;
};

float near_clip_plane(property_viewport_transfer pvt)
{
    return pvt.near_far_viewnear_viewfar.x;
}

float far_clip_plane(property_viewport_transfer pvt)
{
    return pvt.near_far_viewnear_viewfar.y;
}

float view_far_clip_plane(property_viewport_transfer pvt)
{
    return pvt.near_far_viewnear_viewfar.w;
}


float view_near_clip_plane(property_viewport_transfer pvt)
{
    return pvt.near_far_viewnear_viewfar.z;
}

float3 property_viewport_transfer_eye_world_position(property_viewport_transfer pvt)
{
    return pvt.world_to_eye._14_24_34;
}

float3 property_viewport_transfer_eye_world_direction(property_viewport_transfer pvt)
{
    return normalize(pvt.world_to_eye._13_23_33);
}

#endif