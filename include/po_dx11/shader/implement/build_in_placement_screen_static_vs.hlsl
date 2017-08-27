#include "../include/build_in_standard_input_type.hlsli"

void main(in standard_ia_input input, out standard_ps_input output)
{
    float4 lp = input.poisition / input.poisition.w;
    output.position_local = lp;
    float4 wp = mul(mat.local_to_world, lp);
    wp = wp / wp.w;
    output.position_world = wp;
    output.position_view = wp;
    output.position_sv = wp;
    output.uv_screen = cast_sv_position_xy_to_uv_screen(wp);
    output.uv = input.uv;
}