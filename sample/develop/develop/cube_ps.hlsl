struct vs_output
{
	float4 poi:SV_POSITION;
	float3 col:DISSUSE;
	float3 ori_poi : ORI_POSITION;
};

struct ps_out
{
	float4 col : SV_TARGET0;
	float4 poi : SV_TARGET1;
};

ps_out main(in vs_output vo)
{
	ps_out po;
	po.col = float4(vo.col, 1.0);
	po.poi = float4(vo.poi.x / 1024.0 / vo.poi.w, vo.poi.y / 768.0 / vo.poi.w, vo.poi.z / vo.poi.w, 1.0);
	return po;
}












/*
cbuffer view
{
	float4x4 view;
	float4x4 pro;
	float nearz;
};

texture3D cloud : register(t[0]);
texture3D shadow : register(t[1]);

struct ps_input
{
	float4 poi:SV_POSITION;
	float3 col:DISSUSE;
	float3 ori_poi : ORI_POSITION;
	float3 world_poi : WOR_POSITION;
};


float cloud_sam(float3 poi)
{
	float3 cur = poi + 0.5;
	int3 p = cur * 255;
	float D = cloud[p].x;
	return D;
}

float shado_sam(float3 poi)
{
	float3 cur = poi + 0.5;
	int3 p = cur * 255;
	float D = shadow[p].x;
	return D;
}

float4 main(in ps_input i) : SV_TARGET
{
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
	//return float4(shadow[uint3(0, 0, 0)].x, 0.0, 0.0, 1.0);
}
*/