#include "../include/build_in_placement_screen_inout.hlsli"

placement_screen_vs_output main(uint pos : SCREEN_INDEX)
{
    placement_screen_vs_output o;
    float2 screen[4] =
    {
        float2(-1.0, 1.0),
        float2(-1.0, -1.0),
        float2(1.0, -1.0),
        float2(1.0, 1.0)
    };
    float2 tex[4] =
    {
        float2(0.0, 0.0),
        float2(0.0, 1.0),
        float2(1.0, 1.0),
        float2(1.0, 0.0)
    };
    o.position = float4(screen[pos], 0.0, 1.0);
    o.texcoord = tex[pos];
    o.out_position = float4(screen[pos], 0.0, 1.0);
    return o;
}