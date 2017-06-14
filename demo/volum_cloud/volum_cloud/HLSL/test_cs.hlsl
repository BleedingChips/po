
cbuffer CSConstants : register(b0)
{
    float4 NoiseCenter[300];
    float4 PerLinNoiseFactor[4];
};

RWTexture2D<float4> OutputSurface : register(u0);

float ran(float3 co)
{
    return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 42.1897))) * 43758.5453);
}

float cal_random(uint3 poi, float4 rs)
{
   
    /*
    return ran(
		poi * float3(rs.xy, rs.x + rs.y) + float3(rs.zw, rs.z + rs.w)
	);
    */

    return ran(poi);
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

float cal_perlin_noise(uint3 poi)
{


    return perlin_noise(poi, 8, PerLinNoiseFactor[3]) / 4.0
		+ perlin_noise(poi, 16, PerLinNoiseFactor[2]) / 4.0
		+ perlin_noise(poi, 32, PerLinNoiseFactor[1]) / 2.0;
    //+perlin_noise(poi, 64, PerLinNoiseFactor[0]) / 2.0;
}

uint3 cal3(uint3 input)
{
    return uint3(input.x % 256, input.y % 256, input.x / 256 + (input.y / 256) * 16);
}

[numthreads(32, 32, 1)]
void main(uint3 dispatch_thread_id2 : SV_DispatchThreadID)
{

    uint3 dispatch_thread_id = cal3(dispatch_thread_id2);

    float4 last_vertex;
    float3 float_coordinate = (dispatch_thread_id / 255.0) - 0.5; // to -0.5 ~ 0.5;


    float result = 1.0;
    float ran = (cal_perlin_noise(dispatch_thread_id) - 0.2) * 0.2;

    for (uint i = 0; i < 300; ++i)
    {
        float4 target_data = NoiseCenter[i];
        float3 target_center = target_data.xyz;
        float target_length = target_data.w / 2.0;

        float3 to_center_normal = float_coordinate - target_center;
        float to_center_length = length(to_center_normal);
        float compare_target = target_length / 3.0 + sqrt(max(0.0, cal_perlin_noise(dispatch_thread_id) - 0.1)) / 2.0;
        if (to_center_length < 0.05 + ran)
            result = 10/ 255.0;
        else
            result = min(result, 1.0); //0.01 + (to_center_length - compare_target) * 4.0);
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

    //OutputSurface[dispatch_thread_id2.xy] = float4(dispatch_thread_id.x / 64.0, dispatch_thread_id.y / 64.0, dispatch_thread_id.z / 64.0, 1.0);

    OutputSurface[dispatch_thread_id2.xy] = float4(result, 0.0, 0.0, 1.0);
}