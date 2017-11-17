RWTexture3D<float4> OutputTexture : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    OutputTexture[DTid] = float4(100.0, 100.0, 100.0, 100.0);
}