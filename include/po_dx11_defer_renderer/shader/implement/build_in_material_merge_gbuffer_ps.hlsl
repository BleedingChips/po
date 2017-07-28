#include "../include/build_in_standard_input_type.hlsli"
#include "../include/build_in_property_type.hlsli"

Texture2D inputTexture : register(t0);
SamplerState SS : register(s0);

void main(in standard_ps_input o, out standard_ps_output_post output)
{
    output.color =
    //float4(0.5, 0.5, 0.5, 1.0);
    float4(inputTexture.Sample(SS, o.uv).xyz, 1.0);
}