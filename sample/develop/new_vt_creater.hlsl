tbuffer point_location : register(t0)
{
	float3 poi[256];
};
cbuffer point_count
{
	uint point_count;
};
RWTexture3D<float> volum:register(u);

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID)
{
	uint max_count = min(256, point_count);
	float cur = 0.0f;
	for (uint count = 0; count < max_count; ++count)
	{
		cur = max(cur, 1.0 - smoothstep(0.0, 0.4, length(float3(groupID) - poi[count])));
	}
	volum[groupID] = cur;
}
