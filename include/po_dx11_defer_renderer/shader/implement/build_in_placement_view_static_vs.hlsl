#include "../include/build_in_standard_input_type.hlsli"
#include "../include/build_in_property_type.hlsli"

cbuffer b0 : register(b0)
{
    property_screen renderer;
};

cbuffer b1 : register(b1)
{
    transfer_type mat;
};

void main(in standard_ia_input input, out standard_ps_input output)
{
    float4 lp = input.poisition / input.poisition.w;
    output.position_local = lp;
    float4 wp = mul(mat.local_to_world, lp);
    output.position_world = wp;
    float4 vp = mul(renderer.view, wp);
    output.position_view = vp;
    float4 sv_p = mul(renderer.world_to_screen, wp);
    sv_p = sv_p / sv_p.w;
    output.position_sv = sv_p;
    output.uv_screen = cast_position_screen_xy_to_uv_screen(sv_p.xy);
    output.uv = input.uv;
}