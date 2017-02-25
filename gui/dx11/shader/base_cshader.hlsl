struct int_detect
{
	uint3 gid;
	uint3 gtid;
	uint gi;
	uint3 dgi;
};

RWStructuredBuffer<int_detect> BufferOut : register(u0);

[numthreads(8,8,1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	BufferOut[groupID.x * 8 * 8 * 8 + groupID.y * 8 * 8 + groupThreadID.x * 8 + groupThreadID.y].gid = groupID;
	BufferOut[groupID.x * 8 * 8 * 8 + groupID.y * 8 * 8 + groupThreadID.x * 8 + groupThreadID.y].gtid = groupThreadID;
	BufferOut[groupID.x * 8 * 8 * 8 + groupID.y * 8 * 8 + groupThreadID.x * 8 + groupThreadID.y].gi = groupIndex;
	BufferOut[groupID.x * 8 * 8 * 8 + groupID.y * 8 * 8 + groupThreadID.x * 8 + groupThreadID.y].dgi = dispatchThreadID;
}