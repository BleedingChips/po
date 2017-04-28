/*
struct int_detect
{
	uint3 gid;
	uint3 gtid;
	uint gi;
	uint3 dgi;
};

struct inputdata
{
	float yu;
};

inputdata da : register(b0);

RWStructuredBuffer<int_detect> BufferOut : register(u0);

[numthreads(8,8,1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	BufferOut[groupID.x * 8 * 8 * 8 + groupID.y * 8 * 8 + groupThreadID.x * 8 + groupThreadID.y].gid = groupID;
	BufferOut[groupID.x * 8 * 8 * 8 + groupID.y * 8 * 8 + groupThreadID.x * 8 + groupThreadID.y].gtid = groupThreadID;
	BufferOut[groupID.x * 8 * 8 * 8 + groupID.y * 8 * 8 + groupThreadID.x * 8 + groupThreadID.y].gi = groupIndex;
	BufferOut[groupID.x * 8 * 8 * 8 + groupID.y * 8 * 8 + groupThreadID.x * 8 + groupThreadID.y].dgi = dispatchThreadID;
}
*/

RWTexture3D<float> tex :register(u0);
[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float3 poi = (groupID / float3(255.0, 255.0, 255.0) - float3(0.5, 0.5, 0.5)) * 2.0;
	tex[groupID] = saturate(1.0 - length(poi));
}