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

float rand(float3 co)
{
    return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 42.1897))) * 43758.5453);
}

float worley(float3 n, float s)
{
    float dis = 2.0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int z = -1; z <= 1; ++z)
            {
                float3 p = floor(n / s) + float3(x, y, z);
                float rate = rand(p);
                float3 pre_rate = rate + float3(x, y, z) - frac(n / s);
                float d = length(pre_rate);
                if (dis > d)
                {
                    dis = d;
                }
            }
        }
    }
    return 1.0 - dis;
	
}


float4 main(standard_ps_input spi) : SV_TARGET
{
    return
    float4(worley(float3(spi.uv, ps.time / 100000) * 100, 5.0), 0.0, 0.0f, 1.0f);
   // float4(length(r(floor(spi.uv * 100.0 / 5.0)) /*- frac(spi.uv * 100.0 / 5)*/), 0.0, 0.0, 1.0);
}