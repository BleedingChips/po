
struct vs_input
{
    float4 poi : POSITION;
    float2 tex : TEXCOORD;
};

struct vs_output
{
    float4 poi : SV_POSITION;
    float2 tex : TEXCOORD;
};

vs_output main(vs_input pos)
{
    vs_output o;
    o.poi = pos.poi;
    o.tex = pos.tex;
	return o;
}