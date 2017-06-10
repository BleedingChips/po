
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

    float3 RayStep;
{
        float max_factor = max(max(abs(LocalCameraRay.x), abs(LocalCameraRay.y)), abs(LocalCameraRay.z));
        RayStep = LocalCameraRay / max_factor;
    }

    float3 VolumePosition = (LocalPosition / CubeWidth + 0.5) * 63.0;

    uint3 OriginalTexturePosition = VolumePosition;

    uint2 OriginalTexture2DPosition = uint2(
    OriginalTexturePosition.x + (OriginalTexturePosition.z % 8) * 64,
    OriginalTexturePosition.y + (OriginalTexturePosition.z / 8) * 64
     );

    float len = length(RayStep) / 63.0;

    float last_factor = VolumeTexture[OriginalTexture2DPosition].x;

    float s = (1.0 - exp(log(last_factor) * len)) * VolumeTexture[OriginalTexture2DPosition].y * 1.2;
    float l = 0.0;

    for (uint count = 1; count < 63; ++count)
    {
        float3 RayPoint = OriginalTexturePosition + RayStep * count;
        uint3 RayPoint3 = RayPoint;
        uint2 RayPoint2 = uint2( RayPoint3.x + (RayPoint3.z % 8) * 64, RayPoint3.y + (RayPoint3.z / 8) * 64 );
        float now_factor = VolumeTexture[RayPoint2].x;
        l = l + -log((last_factor + now_factor) / 2.0) * len;
        last_factor = now_factor;
        s = s + exp(-l) * (1.0 - exp(log(now_factor) * len)) * VolumeTexture[RayPoint2].y * 1.2;
        if (
        RayPoint.x > 63.0 || RayPoint.x < 0.0 ||
        RayPoint.y > 63.0 || RayPoint.y < 0.0 ||
        RayPoint.z > 63.0 || RayPoint.z < 0.0
        )
            break;
    }

    return float4(s, s, s, exp(-l));
}