
cbuffer AAAA : register(b0)
{
    float4x4 LocalToWorld;
    float4x4 Pro;
}

struct vs_output
{
    float4 poi : SV_POSITION;
    float3 WorldPoi : WorldPosition;
    float3 LocalPosition : LocalPosition;
};

vs_output main(float3 pos : POSITION)
{
    vs_output o;
    float4 World = mul(LocalToWorld, float4(pos, 1.0));
    float4 Screen = mul(Pro, World);
    o.poi = Screen;
    o.WorldPoi = float3(World.xyz) / World.w;
    o.LocalPosition = pos;
    return o;
}