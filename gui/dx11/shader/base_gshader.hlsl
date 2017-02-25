struct Input
{
	float4 poi : SV_POSITION;
	float2 tex : TEXCOORD;
};

[maxvertexcount(3)]
void main(point Input inp[1], inout TriangleStream<Input> ts)
{
	Input tem;
	tem.poi = inp[0].poi + float4(0.0, 0.1, 0.0, 0.0);
	tem.tex = float2(0.5, 0.2);
	ts.Append(tem);

	tem.poi = inp[0].poi + float4(-0.1, 0.0, 0.0, 0.0);
	tem.tex = float2(0.2, 0.5);
	ts.Append(tem);

	tem.poi = inp[0].poi + float4(0.1, 0.0, 0.0, 0.0);
	tem.tex = float2(0.7, 0.5);
	ts.Append(tem);
}