
struct input
{
	float4 poi:POSITION;
};

struct main_output
{
	float4 poi:SV_POSITION;
};

main_output main(input intp)
{
	main_output mo;
	mo.poi = intp.poi;
	return mo;
}