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
	po.col = float4(vo.ori_poi + 0.5, 1.0);
	po.poi = float4(vo.poi.x / 1024.0 / vo.poi.w, vo.poi.y / 768.0 / vo.poi.w, vo.poi.z / vo.poi.w, 1.0);
	return po;
}

