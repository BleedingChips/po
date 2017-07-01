#include "../include/build_in_placement_defer_inout.hlsli"
cbuffer Pro : register(b0)
{
    float4x4 world_to_screen;
    float4x4 screen_to_world;
};

cbuffer Tra : register(b1)
{
    float4x4 local_to_world;
    float4x4 world_to_local;
};

placement_defer_vs_output main(placement_defer_vs_input i)
{
    placement_defer_vs_output o;
    float4 poi = mul(world_to_screen, mul(local_to_world, float4(i.poi, 1.0)));
    o.local_position = i.poi;
    o.out_poisition = poi;
    o.world_position = poi.xyz / poi.w;
    o.texcoord = i.tex;
    return o;
}