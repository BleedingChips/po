RWTexture2D<float4> OutputSurface : register(u0);

cbuffer LightDirection : register(b0)
{
    float3 Dir;
};

uint3 cal3(uint3 input)
{
    return uint3(input.x % 64, input.y % 64, input.x / 64 + (input.y / 64) * 8);
}

uint2 cal2(uint3 input)
{
    return uint2((input.z % 8) * 64 + input.x, (input.z / 8) * 64 + input.y);
}

float3 adject_view_ray(float3 n)
{
    float max_factor = max(max(abs(n.x), abs(n.y)), abs(n.z));
    return n / max_factor;
}

[numthreads(1, 1, 1)]
void main(uint3 dispatch_thread_id2 : SV_DispatchThreadID)
{

    uint3 dispatch_thread_id = cal3(dispatch_thread_id2);

    float3 light = adject_view_ray(Dir);
    float len = length(light) / 63.0;
    float3 poi = dispatch_thread_id;
    float last_sam = OutputSurface[cal2(dispatch_thread_id)].x;
    float l = 0.0;
    for (uint co = 1; co < 256; ++co)
    {
        float3 curpoi = poi + co * light;
        if (
			curpoi.x <= 256.0 && curpoi.x >= 0.0 &&
			curpoi.y <= 256.0 && curpoi.y >= 0.0 &&
			curpoi.z <= 256.0 && curpoi.z >= 0.0
			)
        {
            float now_sam = OutputSurface[cal2(curpoi)].x;
            l = l - log((last_sam + now_sam) / 2.0) * len;
            last_sam = now_sam;
        }
        else
            break;
    }
    OutputSurface[dispatch_thread_id2.xy] = float4(OutputSurface[dispatch_thread_id2.xy].x, exp(-l), 0.0, 1.0);
}