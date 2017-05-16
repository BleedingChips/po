
struct input
{
	float3 poi:POSITION;
	float3 col:DIFFUSE;
};

cbuffer view_matrix
{
	float4x4 local_to_screen;
};

struct vs_output
{
	float4 poi:SV_POSITION;
	float3 col:DISSUSE;
	float3 ori_poi : ORI_POSITION;
	float4 screen_poi : WORLD_POSITION;
};

vs_output main(in input intp)
{
	vs_output o;
	o.ori_poi = intp.poi;
	o.col = intp.col;
	float4 p = mul(local_to_screen, float4(intp.poi, 1.0));
	o.poi = p;
	o.screen_poi = p;

	return o;
}