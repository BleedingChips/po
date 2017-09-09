#include "../include/build_in_standard_input_type.hlsli"
Texture2D tex : register(t0);
SamplerState ss : register(s0);


float4 main(in standard_ps_input spi) : SV_TARGET
{
    return tex.Sample(ss, spi.uv);
}