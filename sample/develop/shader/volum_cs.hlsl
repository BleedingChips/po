


StructuredBuffer<float3> point_location : register(t0);

uint re(uint3 p)
{
	return p.x * 33 * 33 + p.y * 33 + p.z;
}

float rate(float t)
{
	return 1.0 - t;
}

/*
float sam(uint3 poii)
{
	uint3 poi = poii / 32;
	uint o = re(poi);
	uint x = re(poi + uint3(1, 0, 0));
	uint y = re(poi + uint3(0, 1, 0));
	uint z = re(poi + uint3(0, 0, 1));
	uint xy = re(poi + uint3(1, 1, 0));
	uint xz = re(poi + uint3(1, 0, 1));
	uint yz = re(poi + uint3(0, 1, 1));
	uint xyz = re(poi + uint3(1, 1, 1));
	float3 r = (poii % 32) / 32.0;

	float o_x = random_cal[o] * rate(r.x) + random_cal[x] * (1.0 - rate(r.x));
	float y_xy = random_cal[y] * rate(r.x) + random_cal[xy] * (1.0 - rate(r.x));
	float z_xz = random_cal[z] * rate(r.x) + random_cal[xz] * (1.0 - rate(r.x));
	float yz_xyz = random_cal[yz] * rate(r.x) + random_cal[xyz] * (1.0 - rate(r.x));

	float o_x__y_xy = o_x * rate(r.y) + y_xy * (1.0 - rate(r.y));
	float z_xz__yz_xyz = z_xz * rate(r.y) + yz_xyz * (1.0 - rate(r.y));

	float f = o_x__y_xy * rate(r.z) + z_xz__yz_xyz * (1.0 - rate(r.z));
	return f;
}
*/



/*

RWTexture3D<float> volum:register(u[0]);

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID)
{
	float p = 1.0;
	float3 poisi = (groupID / 255.0 - 0.5) * 2.0;
	for (uint i = 0; i < 20; ++i)
	{
		p = min(p, length(point_location[i] - poisi) * 1.5 - 0.2);
		//p = min( p, step(0.3, length(poi[i] - poisi)));
	}




	volum[groupID] = saturate( step(0.8, p) + 0.01 );
}

*/

float rate(float3 p, float3 center)
{
	return 1.0 - saturate(length(p - center));
}

Texture3D<float4> noise:register(t[0]);
StructuredBuffer<float4> poi:register(t[1]);
RWTexture3D<float> volum:register(u[0]);


[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID)
{
	float3 p = (groupID / 255.0) - 0.5;
	float f = 1.0;
	for (uint i = 0; i < 30; ++i)
	{
		float l = length(p - poi[i].xyz);
		if (l < (poi[i].w / 8.0 + (pow(noise[groupID].x, 2.0)) / 4.0))
			f = 0.001;
		else
			f = min(f, 1.0);
	}
	volum[groupID] = f;
}