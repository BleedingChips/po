#include "../include/build_in_standard_input_type.hlsli"
void main(in standard_ia_input input, out standard_ps_input spi)
{
    spi.position_local = input.poisition;
    spi.position_world = input.poisition;
    spi.position_view = input.poisition;
    spi.position_sv = input.poisition;
    spi.uv_screen = cast_sv_position_xy_to_uv_screen(input.poisition);
    spi.uv = input.uv;
}