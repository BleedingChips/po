Texture2D colorMap_ : register(t0); 
SamplerState colorSampler_ : register(s0);
cbuffer inu: register(b1)
{
	float4 color;
	float4 input;
};

struct main_output
{
	float4 poi:SV_POSITION;
	float2 dif:TEXCOORD;
};
float4 main(main_output mo) : SV_TARGET
{
	return  color;
	
	//colorMap_.Sample(colorSampler_, mo.dif);
}