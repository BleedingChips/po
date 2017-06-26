


//StructuredBuffer<float3> point_location : register(t0);

float ran(float3 co)
{
    return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 42.1897))) * 43758.5453);
}

float cal_random(uint3 poi, float4 rs)
{
    return ran(
		poi * float3(rs.xy, rs.x + rs.y) * 10 + float3(rs.zw, rs.z + rs.w)
	);
}

float rate(float t)
{
	//return 1.0 - t;
    return 1.0 - 6 * pow(t, 5) + 15 * pow(t, 4) - 10 * pow(t, 3);
}

float perlin_noise(uint3 poi, uint step, float4 rd)
{
    uint3 o = poi / step;
    uint3 x = o + uint3(1, 0, 0);
    uint3 y = o + uint3(0, 1, 0);
    uint3 z = o + uint3(0, 0, 1);
    uint3 xy = o + uint3(1, 1, 0);
    uint3 xz = o + uint3(1, 0, 1);
    uint3 yz = o + uint3(0, 1, 1);
    uint3 xyz = o + uint3(1, 1, 1);
    float3 r = (poi % step) / float(step);

    float o_x = cal_random(o, rd) * rate(r.x) + cal_random(x, rd) * (1.0 - rate(r.x));
    float y_xy = cal_random(y, rd) * rate(r.x) + cal_random(xy, rd) * (1.0 - rate(r.x));
    float z_xz = cal_random(z, rd) * rate(r.x) + cal_random(xz, rd) * (1.0 - rate(r.x));
    float yz_xyz = cal_random(yz, rd) * rate(r.x) + cal_random(xyz, rd) * (1.0 - rate(r.x));

    float o_x__y_xy = o_x * rate(r.y) + y_xy * (1.0 - rate(r.y));
    float z_xz__yz_xyz = z_xz * rate(r.y) + yz_xyz * (1.0 - rate(r.y));

    float f = o_x__y_xy * rate(r.z) + z_xz__yz_xyz * (1.0 - rate(r.z));
    return f;
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

RWTexture3D<float> volum:register(u0);

cbuffer Data : register(b0)
{
    float4 Cal[20];
    float4 Perlin[4];
}

float cal_perlin_noise(uint3 poi)
{
    return perlin_noise(poi, 8, Perlin[3]) / 8.0
		+ perlin_noise(poi, 16, Perlin[2]) / 8.0
		+ perlin_noise(poi, 32, Perlin[1]) / 4.0;
    +perlin_noise(poi, 64, Perlin[0]) / 2.0;
}

#define SAMPLE_COUNT 20

[numthreads(1, 1, 1)]
void main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
	float4 last_vertex;
	float3 float_coordinate = (dispatch_thread_id / 255.0) - 0.5; // to -0.5 ~ 0.5;
	float result = 1.0;
	float reduce = 1.0;
	for (uint i = 0; i < SAMPLE_COUNT; ++i)
	{
        float4 target_data = Cal[i];
		float3 target_center = target_data.xyz;
		float target_length = target_data.w / 2.0;

		float3 to_center_normal = float_coordinate - target_center;
		float to_center_length = length(to_center_normal);
        float compare_target = target_length / 6.0 + sqrt(max(0.0, cal_perlin_noise(dispatch_thread_id) - 0.2)) / 4.0;
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