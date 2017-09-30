cbuffer B0 : register(b0)
{
    uint2 texture_size;
    uint4 simulate_size;
    float4 value_factor;
}

SamplerState SS : register(s0);
Texture3D InputeTexture : register(t0);
RWTexture2D<uint4> OutputTexture : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint3 simulate_poi = uint3(
    DTid.xy % simulate_size.xy,
    dot(DTid.xy / simulate_size.xy, uint2(1, simulate_size.z))
);
    uint Z = simulate_size.z * simulate_size.w;
    uint3 TotalSize = uint3(simulate_size.xy, Z * 4);

    float3 poi1 = simulate_poi / float3(TotalSize - 1);
    float3 poi2 = (simulate_poi + uint3(0, 0, Z * 1)) / float3(TotalSize - 1);
    float3 poi3 = (simulate_poi + uint3(0, 0, Z * 2)) / float3(TotalSize - 1);
    float3 poi4 = (simulate_poi + uint3(0, 0, Z * 3)) / float3(TotalSize - 1);

    //float4 Value2 = InputeTexture.SampleLevel(SS, poi1, 0).xyzw;

    //float Value1 = dot(InputeTexture.Sample(SS, poi1).xyzw, value_factor);

    
    float4 value = float4(
    //InputeTexture[simulate_poi].x,
    //InputeTexture[simulate_poi + uint3(0, 0, Z * 1)].x,
    //InputeTexture[simulate_poi + uint3(0, 0, Z * 2)].x,
    //InputeTexture[simulate_poi + uint3(0, 0, Z * 3)].x



    
    dot(InputeTexture.SampleLevel(SS, poi1, 0), value_factor),
    dot(InputeTexture.SampleLevel(SS, poi2, 0), value_factor),
    dot(InputeTexture.SampleLevel(SS, poi3, 0), value_factor),
    dot(InputeTexture.SampleLevel(SS, poi4, 0), value_factor)
);


    OutputTexture[DTid.xy] = uint4(clamp(value, 0.0, 1.0) * 255);

}