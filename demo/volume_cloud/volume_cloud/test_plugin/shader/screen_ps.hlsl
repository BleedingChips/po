struct vs_out
{
	float4 poi : SV_POSITION;
	float2 tex : TEXCOORD0;
};

texture2D col : register(t[0]);
texture2D poi: register(t[1]);

sampler normal_sample : register(s[0]);


float4 main(vs_out i) : SV_TARGET
{
	return float4(col.Sample(normal_sample, i.tex).xyz, 1.0);
}
