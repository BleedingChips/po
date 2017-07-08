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

float perlin_noise_rate(float t)
{
    /*
    float r3 = r * r * r;
    float r4 = r3 * r;
    float r5 = r4 * r;
    return 6 * r5 + 15 * r4 - 10 * r3;
    */
    return (t * t * t * (t * (t * 6 - 15) + 10));
    //return t;
}

float perlin_noise(float p, float ran_seed)
{
    float p1 = floor(p);
    float p2 = p1 + 1.0;
    float rate = perlin_noise_rate(p - p1);
    ran_seed = rand(ran_seed);
    float ran_seed2 = ran_seed * ran_seed;

    p1 = rand(ran_seed * p1 + ran_seed2);
    p2 = rand(ran_seed * p2 + ran_seed2);
    return p1 * (1.0 - rate) + p2 * rate;
}

float perlin_noise(float2 p, float2 ran_seed)
{
    float2 o = floor(p);
    float2 rate = p - o;
    rate = float2(perlin_noise_rate(rate.x), perlin_noise_rate(rate.y));

    float temporary1 = rand(ran_seed);
    ran_seed = float2(temporary1, temporary1);
    float2 rand_seed2 = ran_seed * ran_seed;

    float temporary2;
    float temporary3;

    temporary1 = rand(ran_seed * o + rand_seed2);
    temporary2 = rand(ran_seed * (o + float2(1.0, 0.0)) + rand_seed2);

    temporary3 = temporary1 * (1.0 - rate.x) + temporary2 * rate.x;

    temporary1 = rand(ran_seed * (o + float2(0.0, 1.0)) + rand_seed2);
    temporary2 = rand(ran_seed * (o + float2(1.0, 1.0)) + rand_seed2);

    temporary1 = temporary1 * (1.0 - rate.x) + temporary2 * rate.x;

    return temporary3 * (1.0 - rate.y) + temporary1 * rate.y;
}

float perlin_noise(float3 p, float3 ran_seed)
{

    float3 o = floor(p);
    float3 rate = p - o;
    rate = float3(perlin_noise_rate(rate.x), perlin_noise_rate(rate.y), perlin_noise_rate(rate.z));

    float temporary1 = rand(ran_seed);
    ran_seed = float3(temporary1, temporary1, temporary1);
    float3 rand_seed2 = ran_seed * ran_seed;

    float temporary2;

    temporary1 = rand(ran_seed * (o) + rand_seed2);
    temporary2 = rand(ran_seed * (o + float3(1.0, 0.0, 0.0)) + rand_seed2);

    float temporary3;

    temporary3 = temporary1 * (1.0 - rate.x) + temporary2 * rate.x; // {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}

    temporary1 = rand(ran_seed * (o + float3(0.0, 1.0, 0.0)) + rand_seed2);
    temporary2 = rand(ran_seed * (o + float3(1.0, 1.0, 0.0)) + rand_seed2);

    temporary1 = temporary1 * (1.0 - rate.x) + temporary2 * rate.x; // {0.0, 1.0, 0.0}, {1.0, 1.0, 0.0}

    float temporary4;
    temporary4 = temporary3 * (1.0 - rate.y) + temporary1 * rate.y; // {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 1.0, 0.0}

    temporary1 = rand(ran_seed * (o + float3(0.0, 0.0, 1.0)) + rand_seed2);
    temporary2 = rand(ran_seed * (o + float3(1.0, 0.0, 1.0)) + rand_seed2);

    temporary3 = temporary1 * (1.0 - rate.x) + temporary2 * rate.x; // {0.0, 0.0, 1.0}, {1.0, 0.0, 1.0}

    temporary1 = rand(ran_seed * (o + float3(0.0, 1.0, 1.0)) + rand_seed2);
    temporary2 = rand(ran_seed * (o + float3(1.0, 1.0, 1.0)) + rand_seed2);

    temporary1 = temporary1 * (1.0 - rate.x) + temporary2 * rate.x; // {0.0, 1.0, 1.0}, {1.0, 1.0, 1.0}

    temporary3 = temporary3 * (1.0 - rate.y) + temporary1 * rate.y; // {0.0, 0.0, 1.0}, {1.0, 0.0, 1.0},  {0.0, 1.0, 1.0}, {1.0, 1.0, 1.0}

    return temporary4 * (1.0 - rate.z) + temporary3 * rate.z;
}
#endif