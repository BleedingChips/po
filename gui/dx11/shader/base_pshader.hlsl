Texture2D colorMap_ : register(t0); 
SamplerState colorSampler_ : register(s0);
struct main_output
{
	float4 poi:SV_POSITION;
	float2 dif:TEXCOORD;
};
float4 main(main_output mo) : SV_TARGET
{
	return  
	
	colorMap_.Sample(colorSampler_, mo.dif);
}