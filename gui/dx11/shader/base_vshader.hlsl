
struct input
{
	float4 poi:POSITION;
	float2 dif:TEXCOORD0;
};

struct main_output
{
	float4 poi:SV_POSITION;
	float2 dif:TEXCOORD0;
};

main_output main(input intp)
{
	main_output mo;
	mo.poi = intp.poi;
	mo.dif = intp.dif;
	return mo;
}