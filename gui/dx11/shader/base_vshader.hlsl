
struct input
{
	float4 poi:POSITION0;
	float3 col:DIFFUSE;
};

cbuffer lalala
{
	float4x4 view;
	float4x4 pro;
};

struct main_output
{
	float4 poi:SV_POSITION;
	float3 col:DIFFUSE;
};

main_output main(in input intp)
{
	main_output mo;
	mo.poi = mul(pro, mul(view, intp.poi));
		//mul(pro, intp.poi - float4(0.0, 0.0, -1.0, 0.0));
	mo.col = intp.col;
	return mo;
}