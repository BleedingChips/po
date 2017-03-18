struct pinput
{
	float4 poi:SV_POSITION;
	float2 tex:TEXCOORD;
};

texture2D debug : register(t0);
SamplerState debug_sample : register(s0);

float4 main(pinput inp):SV_TARGET
{
	//return float4(1.0,1.0,1.0,1.0);
	return float4(debug.Sample(debug_sample, inp.tex).xyz, 1.0);
}