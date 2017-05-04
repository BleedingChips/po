tbuffer point_location : register(t0)
{
	float3 poi[20];
};
RWTexture3D<float> volum:register(u);

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID)
{
	float p = 1.0;
	float3 poisi = (groupID / 255.0 - 0.5) * 2.0;
	for (uint i = 0; i < 20; ++i)
	{
		p = min(p, length(poi[i] - poisi) * 1.5 + 0.1);
		//p = min( p, step(0.3, length(poi[i] - poisi)));
	}
	volum[groupID] = saturate(p);
}
