#define cMult 0.0001002707309736288
#define aSubtract 0.2727272727272727


float ran(float3 co)
{
	return frac(sin(dot(co.xyz, float3(12.9898, 78.233, 42.1897))) * 43758.5453);
}

texture3D volum:register(t[0]);

cbuffer random_factor : register(b[0])
{
	float4 factor1;
	float4 factor2;
};

RWTexture3D<float> output : register(u[0]);

[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID)
{
	float3 ra = float3(
		ran((float4(groupID, 0.0) * factor1 + factor2).xyz) * 2.0 - 1.0,
		ran((float4(groupID, 0.0) * factor2 + factor1).xyz) * 2.0 - 1.0,
		ran((float4(groupID, 0.0)).xyz) * 2.0 - 1.0
		);
	
	int2 s_x_loca = int2(-1, 1);
	if (groupID.x == 0) s_x_loca.x = 0;
	if (groupID.x == 255) s_x_loca.y = 0;
	int2 s_y_loca = int2(-1, 1);
	if (groupID.y == 0) s_y_loca.x = 0;
	if (groupID.y == 255) s_y_loca.y = 0;
	int2 s_z_loca = int2(-1, 1);
	if (groupID.z == 0) s_z_loca.x = 0;
	if (groupID.z == 255) s_z_loca.y = 0;
	float2 s_x = (log(volum[groupID + uint3(s_x_loca.y, 0, 0)].x) - log(volum[groupID + uint3(s_x_loca.x, 0, 0)].x)) * ra.x;
	float2 s_y = (log(volum[groupID + uint3(0, s_y_loca.y, 0)].x) - log(volum[groupID + uint3(0, s_y_loca.x, 0)].x)) * ra.y;
	float2 s_z = (log(volum[groupID + uint3(0, 0, s_z_loca.y)].x) - log(volum[groupID + uint3(0, 0, s_z_loca.x)].x)) * ra.z;
	output[groupID] = exp(s_x + s_y + s_z + log(volum[groupID].x));
}