cbuffer B0 : register(b0)
{
    uint2 texture_size;
    uint4 simulate_size;
    float4 value_factor;
}

Texture3D InputeTexture : register(t0);
RWTexture2D<uint4> OutputTexture : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint2 pre_index = DTid.xy / simulate_size.xy;
    uint3 simulate_poi = uint3(
    DTid.xy % simulate_size.xy,
		pre_index.x + pre_index.y * simulate_size.z
);

	uint3 TotalSize = uint3(0, 0, simulate_size.z * simulate_size.w);
    
    float4 value = float4(
    
    dot(InputeTexture[simulate_poi], value_factor),
		dot(InputeTexture[simulate_poi + TotalSize], value_factor),
		dot(InputeTexture[simulate_poi + 2 * TotalSize], value_factor),
		dot(InputeTexture[simulate_poi + 3 * TotalSize], value_factor)
);


    OutputTexture[DTid.xy] = uint4(clamp(value, 0.0, 1.0) * 255);

}