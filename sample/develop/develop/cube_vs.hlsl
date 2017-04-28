
struct input
{
	float3 poi:POSITION;
	float3 col:DIFFUSE;
};

cbuffer view_matrix
{
	float4x4 view;
	float4x4 pro;
};

struct vs_output
{
	float4 poi:SV_POSITION;
	float3 col:DISSUSE;
	float3 ori_poi : ORI_POSITION;
	float3 world_poi : WOR_POSITION;
};

vs_output main(in input intp)
{
	vs_output o;
	float4 wor = mul(view, float4(intp.poi, 1.0));
	o.poi = mul(pro, wor);
	o.world_poi = wor.xyz / wor.w;
	o.col = intp.col;
	o.ori_poi = intp.poi;
	return o;
}