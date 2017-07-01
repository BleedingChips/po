#ifndef DX11_NOISE_INCLUDE_HLSLI
#define DX11_NOISE_INCLUDE_HLSLI

float rand(float c)
{
    return frac(sin(dot(float2(c, 11.1 * c), float2(12.9898, 78.233))) * 43758.5453);
}

float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float rand(float3 co)
{
    return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 42.1897))) * 43758.5453);
}

float perlin_rate(float r)
{
    return 1.0 - 6 * pow(r, 5) + 15 * pow(r, 4) - 10 * pow(r, 3);
}

float noise(float p, float ran_seed)
{
    float p1 = floor(p);
    float p2 = p1 + 1.0;
    float rate = perlin_rate(p - p1);
    ran_seed = rand(ran_seed);
    float ran_seed2 = ran_seed * ran_seed;
    float value_p1 = rand(ran_seed * p1 + ran_seed2 );
    float value_p2 = rand(ran_seed * p2 + ran_seed2 );
    return value_p1 * rate + value_p2 * (1.0 - rate);
}

float noise(float2 p, float2 ran_seed)
{
    float2 o = floor(p);
    float2 rate = p - o;
    rate = float2(perlin_rate(rate.x), perlin_rate(rate.y));
    float2 x = o + float2(1.0, 0.0);
    float2 y = o + float2(0.0, 1.0);
    float2 xy = o + float2(1.0, 1.0);
    ran_seed = rand(ran_seed);
    float ran_seed2 = ran_seed * ran_seed;
    o = rand(ran_seed * o + ran_seed);
    x = rand(ran_seed * x + ran_seed);
    xy = rand(ran_seed * xy + ran_seed);
    y = rand(ran_seed * y + ran_seed);
    return (o * rate.x + x * (1.0 - rate.x)) * rate.y +
    (y * rate.x + xy * (1.0 - rate.x)) * (1.0 - rate.y);
}

float noise(float3 p, float3 ran_seed)
{

    float3 value[8] =
    {
      float3(0.0, 0.0, 0.0),
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(1.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0),
        float3(1.0, 0.0, 1.0),
        float3(0.0, 1.0, 1.0),
        float3(1.0, 1.0, 1.0),
    };

    ran_seed = rand(ran_seed);
    float ran_seed2 = ran_seed * ran_seed;

    float3 o = floor(p);
    float3 rate = p - o;
    rate = float3(perlin_rate(rate.x), perlin_rate(rate.y), perlin_rate(rate.z));

    uint count = 0;
    for (count = 0; count < 8; ++count)
        value[count] = rand(ran_seed * (o + value[count]) + ran_seed);

    for (count = 0; count < 4; ++count)
        value[count] = value[count * 2] * rate.x + value[count * 2 + 1] * (1.0 - rate.x);

    for (count = 0; count < 2; ++count)
        value[count] = value[count * 2] * rate.y + value[count * 2 + 1] * (1.0 - rate.y);

    return value[0] * rate.z + value[1] * (1.0 - rate.z);
}
#endif