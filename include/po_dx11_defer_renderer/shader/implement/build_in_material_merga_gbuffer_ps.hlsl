#include "../include/build_in_placement_screen_inout.hlsli"
Texture2D inputTexture : register(t0);
SamplerState SS : register(s0);

float4 main(placement_screen_vs_output o) : SV_TARGET
{
    return inputTexture.Sample(SS, o.texcoord);
}