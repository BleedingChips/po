
Texture3D volum : register(t0);

SamplerState samp : register(s0);

float cloud_sam(float3 poi)
{
	float3 cur = poi + 0.5;
	//int3 p = cur * 255;
	float D = volum.Sample(samp, cur).x;
	return D;
}

float shado_sam(float3 poi)
{
	float3 cur = poi + 0.5;
	int3 p = cur * 255;
    float D = volum.Sample(samp, cur).y;
	return D;
}

struct vs_output
{
	float4 poi:SV_POSITION;
	float3 col:DISSUSE;
	float3 ori_poi : ORI_POSITION;
	float4 screen_poi : WORLD_POSITION;
};

cbuffer ps_buffer:register(b0)
{
	float4x4 view_to_local;
	float nearz;
};

float3 adject_view_ray(float3 n)
{
	float max_factor = max(max(abs(n.x), abs(n.y)), abs(n.z));
	return n / max_factor;
}



float4 main(in vs_output i) : SV_TARGET
{
	float4 eye_poi = mul(view_to_local, float4(0.0, 0.0, 0.0, 1.0));
	float3 eye_vector = -(eye_poi.xyz / eye_poi.w - i.ori_poi);
	float3 eye_local = adject_view_ray(eye_vector.xyz);
	float3 ray_steo = eye_local / 255.0;

	/*
	float4 eye_vector = float4(i.screen_poi.xyz / i.screen_poi.w - float3(0.0, 0.0, 0.0), 0.0);
	float4 eye_vector_local = mul(view_to_local, eye_vector);
	float3 eye_local = adject_view_ray(eye_vector_local.xyz);
	float3 ray_steo = eye_local / 255.0;
	*/

	
	float len = length(ray_steo);
	

	float last_factor = volum.Sample(samp, i.ori_poi + float3(0.5, 0.5, 0.5)).x;
	//float s = (1.0 - last_factor) * shadow.Sample(samp, i.ori_poi + float3(0.5, 0.5, 0.5)).x;
	float s = 0.0;
	float l = 0.0;

	uint count = 1;
	for (; count < 256; ++count)
	{
		float3 p = i.ori_poi + count * ray_steo;
		float now_factor = volum.Sample(samp, p + float3(0.5, 0.5, 0.5)).x;
		l = l + -log((last_factor + now_factor) / 2.0) * len;
		last_factor = now_factor;
        s = s + exp(-l) * (1.0 - exp(log(now_factor) * len)) * volum.Sample(samp, p + float3(0.5, 0.5, 0.5)).y;
		if (abs(p.x) >= 0.50 || abs(p.y) >= 0.50 || abs(p.z) >= 0.50) break;
	}
	return float4(s, s, s, exp(-l));

	/*
	float sc = shadow.Sample(samp, i.ori_poi + float3(0.5, 0.5, 0.5));
	float cc = volum.Sample(samp, i.ori_poi + float3(0.5, 0.5, 0.5));
	float s = (1.0 - cc ) * sc * 0.015;
	float l = 1.0;
	for (; count < 256; ++count)
	{
		float3 p = i.ori_poi + count * ray_steo;
		if (abs(p.x) <= 0.50 && abs(p.y) <= 0.50 && abs(p.z) <= 0.50)
		{
			float sc = shadow.Sample(samp, p + float3(0.5, 0.5, 0.5));
			float cc = volum.Sample(samp, p + float3(0.5, 0.5, 0.5));
			s = s + l * (1.0 - cc) * sc * 0.015;
			l = l * pow(cc, len * 2);
		}
		else
			break;
	}
	*/

	//return float4(1.0 - l, 0.0, 0.0, 0.0);










	//return float4(0.2, 0.2, 0.2, 1.0);
	/*
	float3 eye_ray = i.world_poi - float3(0.0, 0.0, -nearz);
	float3 eye_dir = normalize(mul(view,  float4(eye_ray, 0.0)).xyz);
	float s = 0.0;
	float l = 1.0;
	for (uint count = 0; count < 444; ++count)
	{
		float3 p = i.ori_poi + count * eye_dir / 255.0f;
		if (
			abs(p.x) <= 0.50 &&
			abs(p.y) <= 0.50 &&
			abs(p.z) <= 0.50
			)
		{
			float sc = shado_sam(p);
			float cc = cloud_sam(p);
			s = s + l * (1.0 - cc) * sc * 0.015;
			l = l * pow(cc, 1 / 256.0);
		}
	}
	return float4(s, s, s, l);
	*/
	//return float4(shadow[uint3(0, 0, 0)].x, 0.0, 0.0, 1.0);
}