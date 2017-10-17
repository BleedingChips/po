#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"

#define vec2 float2

cbuffer b2 : register(b0)
{
    property_viewport_transfer ps;
}

float r(float n)
{
    return frac(cos(n * 89.42) * 343.42);
}

float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

vec2 r(vec2 n)
{
    return vec2(r(n.x * 23.62 - 300.0 + n.y * 34.35), r(n.x * 45.13 + 256.0 + n.y * 38.89));
}
float worley(vec2 n, float s, float seed)
{
    float dis = 2.0;
    for (int x = -2; x <= 2; x++)
    {
        for (int y = -2; y <= 2; y++)
        {
            vec2 p = floor(n / s) + vec2(x, y);
            float d = length(r(p + seed) + vec2(x, y) - frac(n / s));
            if (dis > d)
            {
                dis = d;
            }
        }
    }
    return 1.0 - dis;
	
}


float4 main(standard_ps_input spi) : SV_TARGET
{
    return float4(r(floor(spi.uv * 10.0 / 5.0)).x, 0.0, 0.0f, 1.0f);
}