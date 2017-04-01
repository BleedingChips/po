cbuffer FrameUpdate : register(b1)
{
	float4x4 mu;
}


float4 main(in float3 POI : POSITION) : SV_POSITION
{
	return mul(mu, float4(POI, 1.0));
}
