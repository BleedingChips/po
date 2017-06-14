Texture2D inp : register(t0);
SamplerState ss : register(s0);
cbuffer fil
{
    float4 Filter;
};

struct ps_input
{
    float4 poi : SV_POSITION;
    float2 tex : TEXCOORD;
};

float4 main(ps_input i) : SV_TARGET
{
    return Filter * inp.Sample(ss, i.tex);
}