Texture3D InputTexture : register(t0);
RWTexture3D<uint4> OutputTexture : register(u0);

cbuffer b0 : register(b0)
{
    uint3 input_size;
    uint3 output_size;
    float EdgeValue;
    float DistanceMulity;
}

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

}