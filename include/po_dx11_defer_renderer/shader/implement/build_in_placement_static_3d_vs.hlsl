#include "../include/build_in_placement_defer_inout.hlsli"
#include "../include/build_in_property_buffer_type.hlsli"

cbuffer Pro : register(b0)
{
    renderer_3d_buffer_t renderer;
};

cbuffer Tra : register(b1)
{
    transfer_3d_static_buffer_t transfer;
};

placement_defer_vs_output main(placement_defer_vs_input i)
{
    placement_defer_vs_output o;
    float4 world_poi = mul(transfer.local_to_world, float4(i.poi, 1.0));

    float4 screen_poi = mul(renderer.world_to_screen, world_poi);

    o.screen_position = screen_poi.xyz / screen_poi.w;
    o.local_position = i.poi;
    o.out_poisition = screen_poi;
    o.world_position = world_poi.xyz / world_poi.w;
    o.texcoord = i.tex;
    return o;
}