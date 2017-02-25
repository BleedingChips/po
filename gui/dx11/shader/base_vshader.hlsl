
struct input
{
	float4 poi:POSITION;
	float2 dif:TEXCOORD;
	float2 shi:SHIFTING;
};

struct main_output
{
	float4 poi:SV_POSITION;
	float2 dif:TEXCOORD;
};

main_output main(input intp)
{
	main_output mo;
	mo.poi = intp.poi + float4(intp.shi, 0.0, 0.0);
	mo.dif = intp.dif;
	return mo;
}