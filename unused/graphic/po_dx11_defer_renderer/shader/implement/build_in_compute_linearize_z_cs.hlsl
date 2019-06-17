#include "../include/build_in_property_type.hlsli"
Texture2D depth_stencil_texture : register(t0);
RWTexture2D<float> linearize_z : register(u0);

cbuffer b0 : register(b0)
{
    property_screen_static pss;
}




[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float f = pss.far_plane;
    float n = pss.near_plane;
    float nv = pss.viewports_near_far.x;
    float fv = pss.viewports_near_far.y;

    float f_n = pss.far_plane - pss.near_plane;
    linearize_z[DTid.xy] =
    (-2.0 * f * n) / ((2 * depth_stencil_texture[DTid.xy].x - fv - nv) * (f - n) / (fv - nv) - f - n);





    /*
    float A = -(pss.far_plane + pss.near_plane) / (pss.far_plane - pss.near_plane);
    float B = (-2.0 * pss.far_plane * pss.near_plane) / (pss.far_plane - pss.near_plane);
    linearize_z[DTid.xy] = (B) / (A + depth_stencil_texture[DTid.xy].x);
*/
}