struct placement_defer_vs_input
{
    float3 poi : POSITION;
    float2 tex : TEXCOORD;
};

struct placement_defer_vs_output
{
    float4 out_poisition : SV_Position;
    float3 local_position : ORI_POSITION;
    float3 world_position : WOR_POSITION;
    float3 screen_position : SCR_POSITION;
    float2 texcoord : TEXCOORD;
};