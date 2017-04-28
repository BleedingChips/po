
struct input
{
	float4 poi : SV_POSITION;
	float3 col :DIFFUSE;
};


float4 main(in input i) : SV_TARGET
{
	return float4(i.col, 1.0);
	
	//colorMap_.Sample(colorSampler_, mo.dif);
}
