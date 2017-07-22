#include "../include/build_in_standard_input_type.hlsli"
void main(in standard_ia_input input, out standard_ps_input spi)
{
    float4 p = input.poisition / input.poisition.w;
    spi.position_local = p;
    spi.position_world = p;
    spi.position_view = p;
    spi.position_sv = p;
    spi.uv_screen = cast_position_screen_xy_to_uv_screen(p.xy);
    spi.uv = input.uv;
}