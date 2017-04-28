
cbuffer shader_cbuffer
{
	float4x4 inverse_view;
	float sample_step;
	uint3 tex_size;
	float3 light_direction;
};

Texture3D tex :register(t[0]);
RWTexture3D<float> shadow_te : register(u);


[numthreads(1, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	//float3 direct = -normalize(mul(inverse_view, float4(light_direction,0.0)).xyz);
	float3 direct = normalize(float3(0.0, -1.0, 0.0));
	float3 poi = groupID;
	float l = 1.0;
	for (uint co = 1; co < 444; ++co)
	{
		float3 curpoi = poi + co * direct;
		if (
			curpoi.x <= 256.0 && curpoi.x >= 0.0 &&
			curpoi.y <= 256.0 && curpoi.y >= 0.0 &&
			curpoi.z <= 256.0 && curpoi.z >= 0.0
			)
		{
			float cm = tex[curpoi].x;
			l = l * pow(cm, 1.0/256.0);
		}
		else break;
	}
	shadow_te[groupID] = l;
}