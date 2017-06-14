
cbuffer AAAA:register(b0)
{
    float4x4 WorldToLocal;
}

Texture2D volume : register(t0);

float4 CallingFunction(float3 LocalCameraRay,
Texture2D VolumeTexture,
float3 LocalPosition,
float Width);

struct ps_input
{
    float4 poi : SV_POSITION;
    float3 WorldPoi : WorldPosition;
    float3 LocalPosition : LocalPosition;
};

float4 main(ps_input input) : SV_TARGET
{
    float3 Eye = input.WorldPoi - float3(0.0, 0.0, 0.0);
    float4 LocalEyeRay = mul(WorldToLocal, float4(Eye, 0.0));
    float3 LocalEyeRay3 = LocalEyeRay.xyz;
    return CallingFunction(LocalEyeRay3, volume, input.LocalPosition, 0.5);
}

float4 CallingFunction(float3 LocalCameraRay,
Texture2D VolumeTexture,
float3 LocalPosition,
float Width)
{
    float CubeWidth = Width * 2.0;

    int count = 0;

    float3 RayStep;
{
        float max_factor = max(max(abs(LocalCameraRay.x), abs(LocalCameraRay.y)), abs(LocalCameraRay.z));
        RayStep = LocalCameraRay / max_factor;
    }

    float3 VolumePosition = (LocalPosition / CubeWidth + 0.5) * 255.0;

    uint3 OriginalTexturePosition = floor(VolumePosition);
    float3 Rate = VolumePosition - OriginalTexturePosition;

    float2 SampleResult[8];
    uint3 Index[8] =
    {
        uint3(0, 0, 0),
        uint3(1, 0, 0),

        uint3(0, 1, 0),
        uint3(1, 1, 0),

        uint3(0, 0, 1),
        uint3(1, 0, 1),

        uint3(0, 1, 1),
        uint3(1, 1, 1),
    };

    for (count = 0; count < 8; ++count)
    {
        uint3 final = OriginalTexturePosition + Index[count];
        uint2 final2 = uint2(final.x + (final.z % 16) * 256,final.y + (final.z / 16) * 256);
        SampleResult[count] = VolumeTexture[final2].xy;
    }

    float2 final_result =
    /*
    pow(SampleResult[0], (1.0 - Rate.x) * (1.0 - Rate.y) * (1.0 - Rate.z)) *
    pow(SampleResult[1], (Rate.x) * (1.0 - Rate.y) * (1.0 - Rate.z)) *

    pow(SampleResult[2], (1.0 - Rate.x) * (Rate.y) * (1.0 - Rate.z)) *
    pow(SampleResult[3], (Rate.x) * (Rate.y) * (1.0 - Rate.z)) *

    pow(SampleResult[4], (1.0 - Rate.x) * (1.0 - Rate.y) * (Rate.z)) *
    pow(SampleResult[5], (Rate.x) * (1.0 - Rate.y) * (Rate.z)) *

    pow(SampleResult[4], (1.0 - Rate.x) * (Rate.y) * (Rate.z)) *
    pow(SampleResult[5], (Rate.x) * (Rate.y) * (Rate.z));
    */

    
    ((SampleResult[0] * (1.0 - Rate.x) + SampleResult[1] * Rate.x) * (1.0 - Rate.y) +
    (SampleResult[2] * (1.0 - Rate.x) + SampleResult[3] * Rate.x) * Rate.y) * (1.0 - Rate.z) +
    ((SampleResult[4] * (1.0 - Rate.x) + SampleResult[5] * Rate.x) * (1.0 - Rate.y) +
    (SampleResult[6] * (1.0 - Rate.x) + SampleResult[7] * Rate.x) * Rate.y) * Rate.z;

    float len = length(RayStep) / 255.0;

    float last_factor = final_result.x;

    float s = (1.0 - exp(log(last_factor.x) * len)) * final_result.y * 1.2;
    float l = 0.0;

    for (count = 1; count < 255; ++count)
    {

        float3 RayPoint = VolumePosition + RayStep * count;
        uint3 RayPointlast = floor(RayPoint);
        Rate = RayPoint - RayPointlast;
        if (
        RayPoint.x > 255.0 || RayPoint.x < 0.0 ||
        RayPoint.y > 255.0 || RayPoint.y < 0.0 ||
        RayPoint.z > 255.0 || RayPoint.z < 0.0
        )
            break;

        for (uint count2 = 0; count2 < 8; ++count2)
        {
            uint3 final = floor(RayPointlast) + Index[count2];
            uint2 final2 = uint2(final.x + (final.z % 16) * 256, final.y + (final.z / 16) * 256);
            SampleResult[count2] = VolumeTexture[final2].xy;
        }

        float2 Sampled =
        /*
            pow(SampleResult[0], (1.0 - Rate.x) * (1.0 - Rate.y) * (1.0 - Rate.z)) *
    pow(SampleResult[1], (Rate.x) * (1.0 - Rate.y) * (1.0 - Rate.z)) *

    pow(SampleResult[2], (1.0 - Rate.x) * (Rate.y) * (1.0 - Rate.z)) *
    pow(SampleResult[3], (Rate.x) * (Rate.y) * (1.0 - Rate.z)) *

    pow(SampleResult[4], (1.0 - Rate.x) * (1.0 - Rate.y) * (Rate.z)) *
    pow(SampleResult[5], (Rate.x) * (1.0 - Rate.y) * (Rate.z)) *

    pow(SampleResult[4], (1.0 - Rate.x) * (Rate.y) * (Rate.z)) *
    pow(SampleResult[5], (Rate.x) * (Rate.y) * (Rate.z));
        */
        
        ((SampleResult[0] * (1.0 - Rate.x) + SampleResult[1] * Rate.x) * (1.0 - Rate.y) +
        (SampleResult[2] * (1.0 - Rate.x) + SampleResult[3] * Rate.x) * Rate.y) * (1.0 - Rate.z) +
        ((SampleResult[4] * (1.0 - Rate.x) + SampleResult[5] * Rate.x) * (1.0 - Rate.y) +
        (SampleResult[6] * (1.0 - Rate.x) + SampleResult[7] * Rate.x) * Rate.y) * Rate.z;

        uint3 RayPoint3 = floor(RayPoint);
        uint2 RayPoint2 = uint2( RayPoint3.x + (RayPoint3.z % 16) * 256, RayPoint3.y + (RayPoint3.z / 16) * 256 );
        float now_factor = Sampled.x;
        l = l + -log((last_factor + now_factor) / 2.0) * len;
        last_factor = now_factor;
        s = s + exp(-l) * (1.0 - exp(log(now_factor) * len)) * Sampled.y * 1.2;
        
    }

    return float4(s, s, s, exp(-l));
}