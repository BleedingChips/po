#ifndef BUILD_IN_STANDARD_INPUT_TYPE_INCLUDE_HLSLI
#define BUILD_IN_STANDARD_INPUT_TYPE_INCLUDE_HLSLI
struct standard_vs_input
{
    float4 poisition : POSITIONT;
    float2 TexCoord : TEXCOORD;
    uint instance : INSTANCE;
};

struct transfer_3d
{
    float4x4 local_to_world;
    float4x4 world_to_local;
};

struct standard_ps_input
{
    float4 local_position : LOCAL_POSITION;
    float4 world_position : WORLD_POSITION;
    float4 screen_position : SCREEN_POSITION;
    float2 TexCoord : TEXCOORD;
    uint instance : INSTANCE;
};
#endif