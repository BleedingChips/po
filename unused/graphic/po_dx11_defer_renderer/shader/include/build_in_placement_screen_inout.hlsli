struct placement_screen_vs_input
{
    uint poi : SCREEN_INDEX;
};

struct placement_screen_vs_output
{
    float4 out_position : SV_Position;
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
};