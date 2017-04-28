RWTexture3D<float> tex :register(u0);

float rand3(float3 co)
{
	return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 42.1897))) * 43758.5453);
}

float interpolation(float d)
{
	return 6 * pow(d, 5) - 15 * pow(d, 4) + 10 * pow(d, 3);
	//return d;
}

float3 inter_rate(uint3 poi, uint size)
{
	return float3(
		interpolation((poi.x % size) / float(size)),
		interpolation((poi.y % size) / float(size)),
		interpolation((poi.z % size) / float(size))
		);
}

cbuffer balabala
{
	uint count;
};

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{


	uint3 poi = groupID / count;
	float3 rate = inter_rate(groupID, count);

	float inter_x1 = rand3(poi * count) * (1.0 - rate.x) + rand3((poi + uint3(1, 0, 0))*count) * rate.x;
	float inter_x2 = rand3((poi + uint3(0, 1, 0))*count) * (1.0 - rate.x) + rand3((poi + uint3(1, 1, 0))*count) * rate.x;
	float inter_x3 = rand3((poi + uint3(0, 0, 1))*count) * (1.0 - rate.x) + rand3((poi + uint3(1, 0, 1))*count) * rate.x;
	float inter_x4 = rand3((poi + uint3(0, 1, 1))*count) * (1.0 - rate.x) + rand3((poi + uint3(1, 1, 1))*count) * rate.x;

	float inter_y1 = inter_x1 * (1.0 - rate.y) + inter_x2 * rate.y;
	float inter_y2 = inter_x3 * (1.0 - rate.y) + inter_x4 * rate.y;

	float inter_z = inter_y1 * (1.0 - rate.z) + inter_y2 * rate.z;

	tex[groupID] = saturate(inter_z - 0.01);

	//float3 poi = (groupID / float3(255.0, 255.0, 255.0) - float3(0.5, 0.5, 0.5)) * 2.0;
	//tex[groupID] = rand3(poi);
}