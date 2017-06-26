struct vs_in
{
	float2 poi :POSITION;
	float2 tex : TEXCOORD0;
};

struct vs_out
{
	float4 poi : SV_POSITION;
	float2 tex : TEXCOORD0;
};

vs_out main(vs_in i)
{
	vs_out o;
	o.poi = float4(i.poi, 0.5, 1.0);
	o.tex = i.tex;
	return o;
}