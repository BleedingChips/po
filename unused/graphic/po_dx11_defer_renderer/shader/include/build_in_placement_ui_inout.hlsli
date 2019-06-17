struct placement_ui_vs_input
{
    float2 poi : POSITION;
    float2 tex : TEXCOORD;
};

struct placemenet_ui_vs_output
{
    float4 out_poisition : SV_Position;
    float2 local_position : ORI_POSITION;
    float2 texcoord : TEXCOORD;
};