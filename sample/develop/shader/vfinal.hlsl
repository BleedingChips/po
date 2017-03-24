
struct out_ber
{
	float4 hPosition:SV_POSITION;
	float2 texCoords:TEXCOORD;
	float lowResTexCoords : LowRes;
	float4 clip_pos : CLIP;
	float2 depth_texc : DEPTH_TEXCOORD;
};














out_ber main(uint in_index : IND)
{
	out_ber OUT;
	float2 poss[4] =
	{
		{ 1.0,-1.0 },
		{ 1.0, 1.0 },
		{ -1.0,-1.0 },
		{ -1.0, 1.0 },
	};
	float2 pos = poss[in_index];
	OUT.hPosition = float4(pos / 2.0f, 0.0, 1.0);
	OUT.hPosition.z = 0.0;
	OUT.texCoords = 0.5*(float2(1.0, 1.0) + float2(pos.x, -pos.y));

	OUT.lowResTexCoords = OUT.texCoords.xy;
	OUT.clip_pos = float4(-1.0, 1.0, 1.0, 1.0);
	OUT.clip_pos.x += 2.0*OUT.texCoords.x;
	OUT.clip_pos.y -= 2.0*OUT.texCoords.y;
	OUT.depth_texc = OUT.texCoords.xy;
	return OUT;
}