#include "../include/build_in_placement_ui_inout.hlsli"

Texture2D tex : register(t0);
SamplerState ss : register(s0);

float4 main(placemenet_ui_vs_output pdo) : SV_TARGET
{
    //return float4(pdo.local_position / 2.0 + 0.5, 1.0);
    return tex.Sample(ss, pdo.texcoord);

}