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

float worley_noise(float2 n, float multy)
{
    float dis = 2.0;
    for (uint count = 0; count < 9; ++count)
    {
        float2 dir = float2((count / 3) % 3, count % 3) - 1.0;
        float2 p = floor(n / 5.0) + dir;
        float rate = rand(p);
        float2 pre_rate = rate + dir - frac(n / 5.0);
        float d = length(pre_rate) * multy;
        dis = min(dis, d);
    }
    return 1.0 - dis;
}

float fbm_worley_noise(in float2 n, in float multy, in uint octaves, in float frequency, in float lacunarity, in float gain)
{
    float amplitude = gain;
    float total = 0.0;
    for (uint i = 0; i < octaves; ++i)
    {
        total += worley_noise(n * frequency, multy) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return total;
}

float worley_noise(float3 n, float multy)
{
    float dis = 2.0;
    for (uint count = 0; count < 27; ++count)
    {
        float3 dir = float3(count / 9, (count / 3) % 3, count % 3) - 1.0;
        float3 p = floor(n / 5.0) + dir;
        float rate = rand(p);
        float3 pre_rate = rate + dir - frac(n / 5.0);
        float d = length(pre_rate) * multy;
        dis = min(dis, d);
    }
    return 1.0 - dis;
}

float fbm_worley_noise(in float3 n, in float multy, in uint octaves, in float frequency, in float lacunarity, in float gain)
{
    float amplitude = gain;
    float total = 0.0;
    for (uint i = 0; i < octaves; ++i)
    {
        total += worley_noise(n * frequency, multy) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return total;
}

float SimplexNoise(float2 poi)
{
    static const float sqrt_3 = 1.7320508075688772f;
    float2 Equal_Poi = float2(
    dot(poi, float2(-(sqrt_3 - 1) / 2, (sqrt_3 + 1) / 2.0)),
    dot(poi, float2((sqrt_3 + 1) / 2, -(sqrt_3 - 1) / 2.0))
);
    float2 mark = floor(Equal_Poi);
    float2 Rate = Equal_Poi - mark;
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

    RateFinal = RateFinal; // * 3 / 2;

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

float fbm_SimplexNoise(in float2 n, in uint octaves, in float frequency, in float lacunarity, in float gain)
{
    float amplitude = gain;
    float total = 0.0;
    for (uint i = 0; i < octaves; ++i)
    {
        total += SimplexNoise(n * frequency) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return total;
}

float SimplexNoise(float3 poi)
{
    static const float3 mat_r_s = -1.0 / 6.0;
    static const float3 mat_s_r = 1.0 / 3.0;
    static const float3 row_2 = 3.0 / 4.0;

    poi = poi + dot(poi, mat_s_r);
    float3 Mark = floor(poi / 10.0);
    float3 Rate = frac(poi / 10.0);

    float MaxChannle = max(max(Rate.x, Rate.y), Rate.z);
    float3 first = step(MaxChannle, Rate);
    first = first * float3(1.0, 1.0 - first.x, (1.0 - first.x) * (1.0 - first.y));
    float MinChannle = min(min(Rate.x, Rate.y), Rate.z);
    float3 third = step(Rate, MinChannle);
    third = third * float3((1.0 - third.y) * (1.0 - third.z), (1.0 - third.z), 1.0);

    float3 Dir[4] =
    {
        float3(0, 0, 0),
        first,
        float3(1, 1, 1) - third,
        float3(1, 1, 1)
    };

    float Value = 0;
    for (uint count = 0; count < 4; ++count)
    { // 0, 0, 0
        float3 DirResult = Rate - Dir[count];
        float3 Dir2 = float3(DirResult.y, DirResult.z, DirResult.x);
        float distance_2 = dot(row_2, DirResult * DirResult) + -0.5 * dot(DirResult, Dir2);
        distance_2 = distance_2 * 1.5;
        distance_2 = 1.0 - distance_2;
        distance_2 = distance_2 * distance_2 * distance_2;
        Value += clamp(distance_2, 0.0, 1.0) * rand(Mark + Dir[count]);
    }

    return Value;

}

float fbm_SimplexNoise(float3 poi, in uint octaves, in float frequency, in float lacunarity, in float gain)
{
    float amplitude = gain;
    float total = 0.0;
    for (uint i = 0; i < 4; ++i)
    {
        total += SimplexNoise(poi * frequency) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return total;

}


#endif