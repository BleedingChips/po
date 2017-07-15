#include "../../../../../include/po_dx11/shader/include/noise.hlsli"

RWTexture3D<float> outSurface : register(u0);

static const uint n_point = 200;
static const float multy_rate = 2.0;

cbuffer random_data : register(b0)
{
    uint3 size;
    float3 wise_noise_point[n_point];
    float3 perlin_noise_factor[4];
}


void swap(inout float i1, inout float i2)
{
    float tem = i1;
    i1 = i2;
    i2 = tem;
}

void fix_stack(in float d, inout float3 last)
{
    if (d < last.z)
    {
        last.z = d;
        if (last.z < last.y)
        {
            swap(last.z, last.y);
            if (last.y < last.x)
            {
                swap(last.x, last.y);
            }
        }
    }
}

void sample_point(in float3 position, inout float3 last)
{
    for (uint count = 0; count < n_point; ++count)
    {
        float dist = distance(position, wise_noise_point[count]) * multy_rate;
        fix_stack(dist, last);
    }
}

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    const uint nPoint = 100;

    float3 last = float3(1.0, 1.0, 1.0);

    float3 rate = (DTid / float3(size.x -1, size.y -1, size.z -1) - 0.5) * 2.0;
    float color = 1.0;

    sample_point(rate, last);
    /*

    if (rate.x + 1.0 / multy_rate > 1.0)
    {
        float3 rate_current = rate + float3(-2.0, 0.0, 0.0);
        sample_point(rate_current, last);
    }

    if (rate.x - 1.0 / multy_rate < -1.0)
    {
        float3 rate_current = rate + float3(2.0, 0.0, 0.0);
        sample_point(rate_current, last);
    }

    if (rate.y + 1.0 / multy_rate > 1.0)
    {
        float3 rate_current = rate + float3(0.0, -2.0, 0.0);
        sample_point(rate_current, last);
    }

    if (rate.y - 1.0 / multy_rate < -1.0)
    {
        float3 rate_current = rate + float3(0.0, 2.0, 0.0);
        sample_point(rate_current, last);
    }

    if (rate.y + 1.0 / multy_rate > 1.0)
    {
        float3 rate_current = rate + float3(0.0, -2.0, 0.0);
        sample_point(rate_current, last);
    }

    if (rate.y - 1.0 / multy_rate < -1.0)
    {
        float3 rate_current = rate + float3(0.0, 2.0, 0.0);
        sample_point(rate_current, last);
    }

    if (rate.z + 1.0 / multy_rate > 1.0)
    {
        float3 rate_current = rate + float3(0.0, 0.0, -2.0);
        sample_point(rate_current, last);
    }

    if (rate.z - 1.0 / multy_rate < -1.0)
    {
        float3 rate_current = rate + float3(0.0, 0.0, 2.0);
        sample_point(rate_current, last);
    }

    float per = perlin_noise(rate * 2.0, float3(6.233, 0.22, 0.34)) / 2.0 +
    perlin_noise(rate * 16.0, float3(6.233, 0.22, 0.34)) / 4.0 +
    perlin_noise(rate * 32.0, float3(6.233, 0.22, 0.34)) / 8.0 +
    perlin_noise(rate * 64.0, float3(6.233, 0.22, 0.34)) / 8.0;
    */

    outSurface[DTid] = (1.0 - last.x);




    /*
    float3 rate = DTid / float3(size.x, size.y, size.z);

    float2 perlin_noise = float2(noise(rate * 8.0, float3(6.233, 0.22, 0.34)), noise(rate * 16.0, float3(6.3, 2.22, 1.54))) / 2.0 +
    float2(noise(rate * 16.0, float3(6.233, 0.22, 0.34)), noise(rate * 32.0, float3(6.3, 2.22, 1.54))) / 4.0 +
    float2(noise(rate * 32.0, float3(6.233, 0.22, 0.34)), noise(rate * 64.0, float3(6.3, 2.22, 1.54))) / 8.0 +
    float2(noise(rate * 64.0, float3(6.233, 0.22, 0.34)), noise(rate * 128.0, float3(6.3, 2.22, 1.54))) / 8.0;

    float2 center = 0.0;
    for (uint count = 0; count != 100; ++count)
    {
        float d = (1.0 - 2.5 * distance(rate, da[count]));
        float2 d2 = float2(d, d);
        //center = max(center, clamp(d2 + (perlin_noise - 0.5) * 2.0, float2(0.0, 0.0), float2(1.0, 1.0)));
        float2 center2 = distance(rate, da[count]);
        //center = max(center, clamp(1.0 - center2, 0.0, 1.0));
        if (distance(rate, da[count]) < 0.05)
            center2 = 1.0;
        else
            center2 = 1 - (distance(rate, da[count]) - 0.05) * 4.0;
        center = max(center, center2);
    }
    //outSurface[DTid] = float2(r, 0.0);

    float r = DTid.z / float(size.z - 1);
    outSurface[DTid] = (center * perlin_noise * clamp(min(40 * r - 0.05, -20 * r + 18), 0.0, 1.0)).x;
    //outSurface[DTid] = center; //* perlin_noise * clamp(min(40 * r - 0.05, -20 * r + 18), 0.0, 1.0);
   // outSurface[DTid] = 0.5;
*/

}