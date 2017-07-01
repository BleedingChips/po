#include "../include/build_in_placement_defer_inout.hlsli"

Texture2D tex : register(t0);
SamplerState ss : register(s0);

float4 main(placement_defer_vs_output pdo) : SV_TARGET
{
    return float4(pdo.local_position / 2.0 + 0.5, 1.0);
}