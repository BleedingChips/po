#include "../include/build_in_property_type.hlsli"
Texture2D depth_stencil_texture : register(t0);
RWTexture2D<float> linearize_z : register(u0);

cbuffer b0 : register(b0)
{
    property_viewport_transfer pss;
}




[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float f = far_clip_plane(pss);
    float n = near_clip_plane(pss);
    float nv = view_near_clip_plane(pss);
    float fv = view_far_clip_plane(pss);

    float depth = depth_stencil_texture[DTid.xy].x;
    double result = (-2.0 * f * n) / ((2.0 * depth - fv - nv) * (f - n) / (fv - nv) - f - n);
    linearize_z[DTid.xy] = result;


    //linearize_z[DTid.xy] = (-2.0 * f * n) / ((2 * depth_stencil_texture[DTid.xy].x - fv - nv) * (f - n) / (fv - nv) - f - n);
    //linearize_z[DTid.xy] = depth_stencil_texture[DTid.xy].x / 2.0;
    
    /*
    float A = -(pss.far_surface + pss.near_surface) / (pss.far_surface - pss.near_surface);
    float B = (-2.0 * pss.far_surface * pss.near_surface) / (pss.far_surface - pss.near_surface);
    linearize_z[DTid.xy] = (B) / (A + depth_stencil_texture[DTid.xy].x);
*/
}