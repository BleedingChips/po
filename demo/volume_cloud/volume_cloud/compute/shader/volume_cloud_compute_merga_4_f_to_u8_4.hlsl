Texture2D tex1 : register(t0);
Texture2D tex2 : register(t1);
Texture2D tex3 : register(t2);
Texture2D tex4 : register(t4);

cbuffer B0 : register(b0)
{
    float4 factor;
    float4 shift;
}

RWTexture2D<float4> output : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float4 merga = float4(
    tex1[DTid.xy].x,
    tex2[DTid.xy].x,
    tex3[DTid.xy].x,
    tex4[DTid.xy].x
    );

    output[DTid.xy] = factor * merga + shift;
};