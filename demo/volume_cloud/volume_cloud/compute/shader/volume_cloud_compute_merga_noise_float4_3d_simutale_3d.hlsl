Texture2D noise_1 : register(t0);
Texture2D noise_2 : register(t1);
RWTexture2D<uint4> output : register(u0);
cbuffer B0 : register(b0)
{
    uint2 texture_size;
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    output[DTid.xy] = uint4(float4(clamp(noise_1[DTid.xy].x - noise_2[DTid.xy].y, 0.0, 1.0), 0.0, 0.0, 1.0) * 255);
}