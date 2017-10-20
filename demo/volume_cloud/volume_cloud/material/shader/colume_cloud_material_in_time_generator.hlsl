#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"

#define vec2 float2
#define vec3 float3

cbuffer b2 : register(b0)
{
    property_viewport_transfer ps;
}

cbuffer b1 : register(b1)
{
    float Scale;
    float Multy;
}



float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float r(float n)
{
    return frac(cos(n * 89.42) * 343.42);
}

vec2 r(vec2 n)
{
    return vec2(r(n.x * 23.62 - 300.0 + n.y * 34.35), r(n.x * 45.13 + 256.0 + n.y * 38.89));
}

float rand(float3 co)
{
    return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 42.1897))) * 43758.5453);
}

float worley(float3 n, float s, float multy)
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
                float d = length(pre_rate) * multy;
                if (dis > d)
                {
                    dis = d;
                }
            }   
        }
    }
    return 1.0 - dis;
}

float fbm_worley(in float3 n,in float s,in float multy, in uint octaves, in float frequency,in float lacunarity, in float gain)
{
    float amplitude = gain;
    float total = 0.0;
    for (uint i = 0; i < octaves; ++i)
    {
        total += worley(n * frequency, s, multy) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return total;
}

#define MOD3 vec3(.1031,.11369,.13787)

vec3 hash33(vec3 p3)
{
    p3 = frac(p3 * MOD3);
    p3 += dot(p3, p3.yxz + 19.19);
    return -1.0 + 2.0 * frac(vec3((p3.x + p3.y) * p3.z, (p3.x + p3.z) * p3.y, (p3.y + p3.z) * p3.x));
}
float perlin_noise(vec3 p)
{
    vec3 pi = floor(p);
    vec3 pf = p - pi;
    
    vec3 w = pf * pf * (3.0 - 2.0 * pf);
    
    return lerp(
        		lerp(
                	lerp(dot(pf - vec3(0, 0, 0), hash33(pi + vec3(0, 0, 0))),
                        dot(pf - vec3(1, 0, 0), hash33(pi + vec3(1, 0, 0))),
                       	w.x),
                	lerp(dot(pf - vec3(0, 0, 1), hash33(pi + vec3(0, 0, 1))),
                        dot(pf - vec3(1, 0, 1), hash33(pi + vec3(1, 0, 1))),
                       	w.x),
                	w.z),
        		lerp(
                    lerp(dot(pf - vec3(0, 1, 0), hash33(pi + vec3(0, 1, 0))),
                        dot(pf - vec3(1, 1, 0), hash33(pi + vec3(1, 1, 0))),
                       	w.x),
                   	lerp(dot(pf - vec3(0, 1, 1), hash33(pi + vec3(0, 1, 1))),
                        dot(pf - vec3(1, 1, 1), hash33(pi + vec3(1, 1, 1))),
                       	w.x),
                	w.z),
    			w.y);
}



float SimplexNoise(float2 poi, float s)
{
    static const float sqrt_3 = 1.7320508075688772f;
    float2 Equal_Poi = float2(
    dot(poi, float2(-(sqrt_3 - 1) / 2, (sqrt_3 + 1) / 2.0)),
    dot(poi, float2((sqrt_3 + 1) / 2, -(sqrt_3 - 1) / 2.0))
);
    float2 poi2 = Equal_Poi / s;
    float2 mark = floor(poi2);
    float2 Rate = poi2 - mark;
    uint step_index = step(1.0, Rate.x + Rate.y);

    static const float2 grauid[2][3] =
    {
        {
            float2(0, 0),
            float2(1, 0),
            float2(0, 1)
        },
        {
            float2(1, 1),
            float2(1, 0),
            float2(0, 1)
        }
    };

    float2 Range[3] =
    {
        grauid[step_index][0] - Rate,
        grauid[step_index][1] - Rate,
        grauid[step_index][2] - Rate
    };

    float r = 1.0;

    float3 RateFinal = float3(
        (dot(Range[0], Range[0]) + Range[0].x * Range[0].y) * r,
         (dot(Range[1], Range[1]) + Range[1].x * Range[1].y) * r,
         (dot(Range[2], Range[2]) + Range[2].x * Range[2].y) * r
    );

    RateFinal = RateFinal;// * 3 / 2;

    RateFinal = 1.0 - RateFinal;
    RateFinal = RateFinal * RateFinal * RateFinal;

    float3 RateP = max(0.0, RateFinal);
    float3 RandomV = //float3(Equal_Poi, 0.0);
    //float3(grauid[step_index][0], rand(mark + grauid[step_index][1]), rand(mark + grauid[step_index][2]));
    
    float3(rand(mark + grauid[step_index][0]), rand(mark + grauid[step_index][1]), rand(mark + grauid[step_index][2]));
    
    return dot(RateP, RandomV);
    //return RateP.x;
    //return (RateP.x + RateP.y + RateP.z) / 2.0;
    //return (rand(mark + grauid[step_index][0]) + rand(mark + grauid[step_index][1]) + rand(mark + grauid[step_index][2])) / 3;



}


float SimplexNoise(float3 poi, float s)
{
    static const float sqrt_3 = 1.7320508075688772f;

    float3 Block = poi / s;
    float3 Rate = poi - Block;
    float3 GrauidBuffer[3] =
    {
        float3(1, 0, 0), float3(0, 1, 0), float3(0, 1, 0)
    };

    float3 Grauid = float3(step(max(Rate.y, Rate.z), Rate.x), step(max(Rate.x, Rate.z), Rate.y), step(max(Rate.y, Rate.x), Rate.z));




    float3 grauid[5];
    grauid[0] = float3(0, 0, 0);
    grauid[1] = step(max(max(Rate.x, Rate.y), Rate.z), Rate);









    float2 Equal_Poi = float2(
    dot(poi, float2(-(sqrt_3 - 1) / 2, (sqrt_3 + 1) / 2.0)),
    dot(poi, float2((sqrt_3 + 1) / 2, -(sqrt_3 - 1) / 2.0))
);
    float2 poi2 = Equal_Poi / s;
    float2 mark = floor(poi2);
    float2 Rate = poi2 - mark;
    uint step_index = step(1.0, Rate.x + Rate.y);

    static const float2 grauid[2][3] =
    {
        {
            float2(0, 0),
            float2(1, 0),
            float2(0, 1)
        },
        {
            float2(1, 1),
            float2(1, 0),
            float2(0, 1)
        }
    };

    float2 Range[3] =
    {
        grauid[step_index][0] - Rate,
        grauid[step_index][1] - Rate,
        grauid[step_index][2] - Rate
    };

    float r = 2 / 3.0;

    float3 RateFinal = float3(
        (dot(Range[0], Range[0]) + Range[0].x * Range[0].y) * r,
         (dot(Range[1], Range[1]) + Range[1].x * Range[1].y) * r,
         (dot(Range[2], Range[2]) + Range[2].x * Range[2].y) * r
    );

    RateFinal = RateFinal * 3 / 2;

    RateFinal = 1.0 - RateFinal;
    RateFinal = RateFinal * RateFinal * RateFinal;

    float3 RateP = max(0.0, RateFinal);
    float3 RandomV = //float3(Equal_Poi, 0.0);
    //float3(grauid[step_index][0], rand(mark + grauid[step_index][1]), rand(mark + grauid[step_index][2]));
    
    float3(rand(mark + grauid[step_index][0]), rand(mark + grauid[step_index][1]), rand(mark + grauid[step_index][2]));
    
    return dot(RateP, RandomV);
    //return RateP.x;
    //return (RateP.x + RateP.y + RateP.z) / 2.0;
    //return (rand(mark + grauid[step_index][0]) + rand(mark + grauid[step_index][1]) + rand(mark + grauid[step_index][2])) / 3;



}


float4 main(standard_ps_input spi) : SV_TARGET
{
    float3 perlin_uv = float3(spi.uv, 0.0) * 10;
    float3 uv = float3(spi.uv, ps.time / 1000.0) * 10;

    return

    float4(SimplexNoise(spi.uv * 100, Scale), 0.0, 0.0, 1.0);

    //float4(hash33(uv), 1.0);
    float4(perlin_noise(perlin_uv) /** 5 * fbm_worley(uv, Scale, Multy, 3, 0.8, 1.8, 0.5)*/, 0.0, 0.0, 1.0);


    float4(fbm_worley(uv, Scale, Multy, 3, 0.8, 1.8, 0.5), 0.0, 0.0f, 1.0f);
   // float4(length(r(floor(spi.uv * 100.0 / 5.0)) /*- frac(spi.uv * 100.0 / 5)*/), 0.0, 0.0, 1.0);
}