RWTexture3D<float> tex :register(u0);
Texture3D noise1: register(t0);
Texture3D noise2: register(t2);
Texture3D noise3: register(t3);
Texture3D noise4: register(t4);
Texture3D noise5: register(t5);

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

float rand3(float3 co)
{
	return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 42.1897))) * 43758.5453);
}


[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	/*
	tex[groupID] = noise1[groupID].x / 2.0 +
		noise2[groupID].x / 4.0 +
		noise3[groupID].x / 8.0 +
		noise4[groupID].x / 16.0 +
		noise5[groupID].x / 16.0;
		*/

	float dis_rate = 1.0 - saturate(length((groupID / 255.0 - 0.5) * 2.0));
	dis_rate = dis_rate * 2.0;
	uint count = 64;

	uint3 poi = groupID / count;
	float3 rate = inter_rate(groupID, count);

	float inter_x1 = rand3(poi * count) * dis_rate * (1.0 - rate.x) + rand3((poi + uint3(1, 0, 0))*count)* dis_rate * rate.x;
	float inter_x2 = rand3((poi + uint3(0, 1, 0)) * count)* dis_rate * (1.0 - rate.x) + rand3((poi + uint3(1, 1, 0))*count) * dis_rate  * rate.x;
	float inter_x3 = rand3((poi + uint3(0, 0, 1)) * count)* dis_rate * (1.0 - rate.x) + rand3((poi + uint3(1, 0, 1))*count) * dis_rate * rate.x;
	float inter_x4 = rand3((poi + uint3(0, 1, 1)) * count)* dis_rate * (1.0 - rate.x) + rand3((poi + uint3(1, 1, 1))*count) * dis_rate * rate.x;

	float inter_y1 = inter_x1 * (1.0 - rate.y) + inter_x2 * rate.y;
	float inter_y2 = inter_x3 * (1.0 - rate.y) + inter_x4 * rate.y;

	float inter_z = inter_y1 * (1.0 - rate.z) + inter_y2 * rate.z;

	tex[groupID] = saturate(1.0 - inter_z);


	/*
	float3 poi = (groupID / 256.0f - float3(0.5, 0.5, 0.5)) * 2;
	tex[groupID] = saturate(length(poi) - 0.2);
	*/
	
	/*
	uint3 poi = groupID / 64;
	float3 rate = inter_rate(groupID, 64);

	float inter_x1 = noise[poi].x * (1.0 - rate.x) + noise[poi + uint3(1, 0, 0)].x * rate.x;
	float inter_x2 = noise[poi + uint3(0, 1, 0)].x * (1.0 - rate.x) + noise[poi + uint3(1, 1, 0)].x * rate.x;
	float inter_x3 = noise[poi + uint3(0, 0, 1)].x * (1.0 - rate.x) + noise[poi + uint3(1, 0, 1)].x * rate.x;
	float inter_x4 = noise[poi + uint3(0, 1, 1)].x * (1.0 - rate.x) + noise[poi + uint3(1, 1, 1)].x * rate.x;

	float inter_y1 = inter_x1 * (1.0 - rate.y) + inter_x2 * rate.y;
	float inter_y2 = inter_x3 * (1.0 - rate.y) + inter_x4 * rate.y;

	float inter_z = inter_y1 * (1.0 - rate.z) + inter_y2 * rate.z;

	tex[groupID] = saturate(inter_z - 0.01);
	*/
	
}