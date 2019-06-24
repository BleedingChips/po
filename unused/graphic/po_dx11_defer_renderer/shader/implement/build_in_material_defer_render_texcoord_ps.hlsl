#include "../include/build_in_standard_input_type.hlsli"
#include "../include/build_in_property_type.hlsli"

void main(in standard_ps_input pdo, out standard_ps_output_defer spo)
{
    float2 tem = pdo.uv;

    tem.x = abs(step(tem.x, 0.0) - fmod(tem.x, 1.0));
    tem.y = abs(step(tem.y, 0.0) - fmod(tem.y, 1.0));
    spo.color = float4(tem, 0.0, 1.0);
}