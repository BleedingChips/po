#include "../include/build_in_standard_input_type.hlsli"
#include "../include/build_in_property_type.hlsli"

cbuffer b0 : register(b0)
{
    property_viewport_transfer renderer;
};

cbuffer b1 : register(b1)
{
    property_local_transfer mat;
};

void main(in standard_ia_input input, out standard_ps_input output)
{
    float4 wp = mul(mat.local_to_world, input.poisition);
    float4 vp = mul(renderer.world_to_eye, wp);
    float4 sv_p = mul(renderer.world_to_camera, wp);
    output.position_local = input.poisition;
    output.position_world = wp;
    output.position_view = vp;
    output.position_sv = sv_p;
    output.uv_screen = cast_sv_position_xy_to_uv_screen(sv_p);
    output.uv = input.uv;
}