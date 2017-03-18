
struct vinput
{
	float2 pos:POSITION;
	float2 tex:TEXCOORD;
};

struct voutput
{
	float4 pos:SV_POSITION;
	float2 tex:TEXCOORD;
};

voutput main(vinput inp)
{
	voutput tem;
	tem.pos = float4(inp.pos,0.0, 1.0);
	tem.tex = inp.tex;
	return tem;
}