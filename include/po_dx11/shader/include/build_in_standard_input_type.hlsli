#ifndef BUILD_IN_STANDARD_INPUT_TYPE_INCLUDE_HLSLI
#define BUILD_IN_STANDARD_INPUT_TYPE_INCLUDE_HLSLI
struct standard_ia_input
{
    float4 poisition : POSITION;
    float2 uv : TEXCOORD;
};

struct transfer_type
{
    float4x4 local_to_world;
    float4x4 world_to_local;
};

struct standard_ps_input
{
    float4 position_sv : SV_Position;
    float4 position_local : POSITION_LOCAL;
    float4 position_world : POSITION_WORLD;
    float4 position_view : POSITION_VIEW;
    float2 uv_screen : POSITION_SCREEN;
    float2 uv : TEXCOORD;
};

struct standard_ps_output_defer
{
    float4 color : SV_Target;
};

struct standard_ps_output_transparent
{
    float4 color : SV_Target;
};

struct standard_ps_output_post
{
    float4 color : SV_Target;
};

float2 cast_sv_position_xy_to_uv_screen(float4 ps)
{
    float2 in_ps = ps.xy;
    /*(ps.xy / ps.w + 1.0) / 2.0;
    in_ps.y = 1.0 - in_ps.y;*/
    return in_ps;
}

float2 get_uv_screen(float2 uv_screen, float4 sv_position)
{
    return (uv_screen * float2(1.0, -1.0) / sv_position.w + 1.0) / 2.0;
}



#endif