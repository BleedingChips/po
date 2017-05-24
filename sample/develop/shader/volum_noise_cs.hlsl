
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

RWTexture3D<float4> volum_texture:register(u[0]);

cbuffer random_multy:register(b[0])
{
	float4 _128_1;
	float4 _128_2;
	float4 _64_1;
	float4 _64_2;
	float4 _32_1;
	float4 _32_2;
	float4 _16_1;
	float4 _16_2;
};

[numthreads(1, 1, 1)]
void main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
	float height =
		perlin_noise(dispatch_thread_id, 8, _16_1) / 8.0
		+ perlin_noise(dispatch_thread_id, 16, _32_1) / 8.0
		+ perlin_noise(dispatch_thread_id, 32, _64_1) / 4.0;
		+ perlin_noise(dispatch_thread_id, 64, _128_1) / 2.0;
		//+ perlin_noise(dispatch_thread_id, 128, _128_1) / 2.0;

	
	float thick =
		//
		//perlin_noise(dispatch_thread_id, 16, _16_2) / 8.0
		 perlin_noise(dispatch_thread_id, 32, _32_2) / 4.0
		+ perlin_noise(dispatch_thread_id, 64, _64_2) / 4.0
		+ perlin_noise(dispatch_thread_id, 128, _128_2) / 2.0;

	volum_texture[dispatch_thread_id] = float4(height, rate(thick), 0.0, 1.0);
		
}
