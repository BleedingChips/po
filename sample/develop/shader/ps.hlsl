
struct ps_input
{
	float4 poi:SV_POSITION;
	float3 col:DISSUSE;
	float3 ori_poi : ORI_POSITION;
	float3 world_poi : WOR_POSITION;
};

float4 main(ps_input i):SV_TARGET
{
	//return float4(1.0,1.0,1.0,1.0);
	return float4(i.ori_poi + 0.5, 1.0);
}