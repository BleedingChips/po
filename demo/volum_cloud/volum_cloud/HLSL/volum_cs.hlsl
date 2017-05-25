


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

#define SAMPLE_COUNT 30

[numthreads(32, 32, 1)]
void main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
	float4 last_vertex;
	float3 float_coordinate = (dispatch_thread_id / 255.0) - 0.5; // to -0.5 ~ 0.5;
	float result = 1.0;
	float reduce = 1.0;
	for (uint i = 0; i < SAMPLE_COUNT; ++i)
	{
		float4 target_data = poi[i];
		float3 target_center = target_data.xyz;
		float target_length = target_data.w / 2.0;

		float3 to_center_normal = float_coordinate - target_center;
		float to_center_length = length(to_center_normal);
		float compare_target = target_length / 6.0 + sqrt(max(0.0, noise[dispatch_thread_id].x - 0.2)) / 4.0;
		if (to_center_length < compare_target)
			result = 0.01;
		else
			result = min(result, 0.01 + (to_center_length - compare_target) * 2.0);
		/*
		float4 target_ver = float4(normalize(to_center_normal), to_center_length - compare_target);
		if (i == 0) last_vertex = target_ver;
		else
		{
			float dot_result = dot(last_vertex.xyz, target_ver.xyz);
			reduce = min(reduce, saturate(dot_result * step(0.0, dot_result) * step(0, last_vertex.w) * last_vertex.w * target_ver.w * step(target_ver.w) * 2.0));
			if (target_ver.w < last_vertex.w) last_vertex = target_ver;
		}*/
		
	}

	volum[dispatch_thread_id] = result; // max(0.01, result - reduce);


	/*
	//sample_count = 30;
	float3 ver[SAMPLE_COUNT];
	float3 p = (groupID / 255.0) - 0.5;
	float f = 1.0;
	for (uint i = 0; i < SAMPLE_COUNT; ++i)
	{
		float target_length = poi[i].w;
		float l = length(p - poi[i].xyz);
		//ver[SAMPLE_COUNT] = (p - poi[i].xyz) * 
		float target = (poi[i].w / 6.0 + sqrt(noise[groupID].x) / 6.0);
		if (l < target)
			f = 0.01;// noise[groupID].y;
		else
			f = min(f, 0.01 + (l - target) * 4);
	}
	volum[dispatch_thread_id] = f;
	*/
}