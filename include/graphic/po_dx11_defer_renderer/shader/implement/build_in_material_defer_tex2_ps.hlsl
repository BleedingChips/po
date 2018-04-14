#include "../include/build_in_standard_input_type.hlsli"
#include "../include/build_in_property_type.hlsli"

Texture2D tex : register(t0);
SamplerState ss : register(s0);

void main(in standard_ps_input spi, out standard_ps_output_defer sp)
{
    sp.color = 
    //float4(0.5, 0.5, 0.5, 0.5);
    tex.Sample(ss, spi.uv);
}