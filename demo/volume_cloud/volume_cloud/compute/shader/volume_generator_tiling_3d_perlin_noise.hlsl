#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

RWTexture3D<float> output : register(u0);
RWTexture3D<float> output2 : register(u1);

float rand_6(float3 poi, float3 poi2)
{
    return rand(float2(rand(poi), rand(poi2)));
}

float Rate(float t)
{
    return (t * t * t * (t * (t * 6 - 15) + 10));
}

float Generator6DPerlinNoise(float3 poi, float3 poi2)
{
    static const float Block = 0.4;

    float3 Mark1 = floor(poi / Block);
    float3 Mark2 = floor(poi2 / Block);

    float3 Rate1 = frac(poi / Block);
    float3 Rate2 = frac(poi2 / Block);

    float RandomValue[64];

    for (uint count = 0; count < 64; ++count)
    {
        float3 Array1 = float3(count / 32, (count / 16) % 2, (count / 8) % 2);
        float3 Array2 = float3((count / 4) % 2, (count / 2) % 2, count % 2);
        RandomValue[count] = rand_6(Mark1 + Array1, Mark2 + Array2);
    }

    for (count = 0; count < 32; ++count)
        RandomValue[count * 2] = lerp(RandomValue[count * 2], RandomValue[count * 2 + 1], Rate(Rate2.z));

    for (count = 0; count < 16; ++count)
        RandomValue[count * 4] = lerp(RandomValue[count * 4], RandomValue[count * 4 + 1 * 2], Rate(Rate2.y));

    for (count = 0; count < 8; ++count)
        RandomValue[count * 8] = lerp(RandomValue[count * 8], RandomValue[count * 8 + 1 * 4], Rate(Rate2.x));
    
    for (count = 0; count < 4; ++count)
        RandomValue[count * 16] = lerp(RandomValue[count * 16], RandomValue[count * 16 + 1 * 8], Rate(Rate1.z));

    for (count = 0; count < 2; ++count)
        RandomValue[count * 32] = lerp(RandomValue[count * 32], RandomValue[count * 32 + 1 * 16], Rate(Rate1.y));

    RandomValue[0] = lerp(RandomValue[0], RandomValue[32], Rate(Rate1.x));
    return RandomValue[0];
}

float P(float3 Poi, float s)
{
    return Generator6DPerlinNoise(sin(Poi * 3.141592653 * 2.0) * s, cos(Poi * 3.141592653 * 2.0) * s);
}

float fbm_P(float3 uv, float s, in uint octaves, in float frequency, in float lacunarity, in float gain)
{
    float amplitude = gain;
    float total = 0.0;
    for (uint i = 0; i < octaves; ++i)
    {
        total += P(uv * frequency, s) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return total;
}

float TilessWorley(float3 uv, uint s, float Length)
{
    uv = fmod(uv, 1.0);
    float dis = 2.0;
    for (uint count = 0; count < 27; ++count)
    {
        float3 dir = float3(count / 9, (count / 3) % 3, count % 3) - 1.0;
        float3 p = floor(uv * s) + dir;
        p = lerp(s - 1, p, step(0.0, p));
        p = lerp(0.0, p, step(p, s - 1));
        float rate = rand(p);
        float3 pre_rate = rate + dir - frac(uv * s);
        float d = length(pre_rate) * Length;
        dis = min(dis, d);
    }
    return 1.0 - dis;
}



float fbm_TilessWorley(float3 uv, uint s, float Length, in uint octaves, in float frequency, in float lacunarity, in float gain)
{
    float amplitude = gain;
    float total = 0.0;
    for (uint i = 0; i < octaves; ++i)
    {
        total += TilessWorley(uv * frequency, s, Length) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return total;
}

/*
float Worley(float3 poi, float3 poi2, float Length)
{
    static const float Block = 2.0;
    float Dis = 1.0;
    for (uint count = 0; count < 800; ++ count)
    {
        float3 P1 = float3(B[count * 2]);
        float3 P2 = float3(B[count * 2 + 1]);
        float3 L1 = poi - P1;
        float3 L2 = poi2 - P2;
        float Length2 = dot(L1, L1) + dot(L2, L2);
        Dis = min(Length2, Dis);
    }
    return 1.0 - Dis;
}

float Worley(float3 poi, float Length)
{
    float Dis = 1.0;
    for (uint count = 0; count < 400; ++count)
    {
        float3 P1 = float3(B[count * 2]);
        float3 L1 = poi - P1;
        float Length2 = length(L1) * Length;
        Dis = min(Length2, Dis);
    }
    return 1.0 - Dis;
}

float PW(float3 Poi, float s, float Length)
{
    return Worley(sin(Poi * 3.141592653 * 2.0) * s, cos(Poi * 3.141592653 * 2.0) * s, Length);
}
*/

float SimplexNoiseN(float3 poi, uint s)
{
    static const float3 mat_r_s = -1.0 / 6.0;
    static const float3 mat_s_r = 1.0 / 3.0;
    static const float3 row_2 = 3.0 / 4.0;

    poi = poi + dot(poi, mat_s_r);
    float3 Mark = floor(poi * s);
    float3 Rate = frac(poi * s);

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
        float3 Add = Mark + Dir[count];
        Add = Add + dot(Add, mat_r_s);
        Add = lerp(s - 1, Add, step(0.0, Add));
        Add = lerp(0.0, Add, step(Add, s - 1));
        Value += clamp(distance_2, 0.0, 1.0) * rand(Add);
    }

    return Value;

}

float fbm_SimplexNoiseN(float3 poi, uint s, in uint octaves, in float frequency, in float lacunarity, in float gain)
{
    float amplitude = gain;
    float total = 0.0;
    for (uint i = 0; i < 4; ++i)
    {
        total += SimplexNoiseN(poi * frequency, s) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return total;

}



[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 Poi = DTid / float3(255, 255, 255);
    Poi.z += 0.475;
        //output[DTid] = SimplexNoiseN(Poi, 10);
    output[DTid] =  fbm_SimplexNoise(Poi * 80 + 0.34325, 3, 0.75, 1.75, 0.5);// * fbm_TilessWorley(Poi, 9, 1.0, 4, 0.8, 1.8, 0.5);
        //output2[DTid] = TilessWorley(Poi, 5, 1.0);
    output2[DTid] = fbm_worley_noise(Poi * 40, 1.0, 6, 0.8, 1.8, 0.5);
    //output3[DTid] = fbm_TilessWorley(Poi, 8, 1.0, 4, 0.8, 1.8, 0.5);
    //output4[DTid] = Worley(Poi, 10.0);
    //output5[DTid] = SimplexNoise(Poi * 100);
        /*
        float Final = 0.0;
        float gain = 0.5;
        float frequency = 0.8;
        float lacunarity = 1.8;

        float amplitude = gain;
        float total = 0.0;
        for (uint count = 0; count < 3; ++count)
        {
            total += P(Poi * frequency, 5.0) * amplitude;
            frequency *= lacunarity;
            amplitude *= gain;
        }
        output[DTid] = total;
*/
}