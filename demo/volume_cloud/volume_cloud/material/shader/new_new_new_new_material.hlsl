#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

Texture2D tex : register(t0);
SamplerState ss : register(s0);

float4 EncodeFloatRGBA(float v)
{
    float4 enc = float4(1.0f, 255.0f, 65025.0f, 16581375.0f) * v;
    enc = frac(enc);
    enc -= enc.yzww * float4(1 / 255.0f, 1 / 255.0f, 1 / 255.0f, 0);
    return enc;
}

float DecodeFloatRGBA(float4 rgba)
{
    return dot(rgba, float4(1, 1 / 255.0f, 1 / 65025.0f, 1 / 16581375.0f));
}

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{
    float4 Value = tex.Sample(ss, input.uv);
    float FinalValue = max(min(Value.x, Value.y), min(max(Value.x, Value.y), Value.z));
    
    
    FinalValue = (FinalValue - 0.5) / 2.0;


    /*
    float3 Dot = step(0.005, Value);
    float FinalValue = dot(Dot, 1);
    if (FinalValue >= 2.0)
        output.color = float4(1.0, 1.0, 1.0, 1.0);
    else
        output.color = float4(0.0, 0.0, 0.0, 1.0);
    */

    if (FinalValue >= 0.01)
        output.color = float4(1.0, 1.0, frac(input.uv.x * 100), 1.0);
    else if (FinalValue >= 0.0)
        output.color = float4(1.0, 0.0, frac(input.uv.x * 100), 1.0);
    else
        output.color = float4(0.0, 0.0, frac(input.uv.x * 100), 1.0);
}