#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11_defer_renderer/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11_defer_renderer/shader/include/build_in_standard_input_type.hlsli"
cbuffer b0 : register(b0)
{
    property_rendder_2d_for_3d pp;
}
cbuffer b1 : register(b1)
{
    property_transfer mat;
}
cbuffer b2 : register(b2)
{
    property_screen ps;
}

Texture2D ten : register(t0);

Texture2D linearize_z : register(t1);
SamplerState ss;


float4 implement(float3 min_width, float3 max_width, float3 ray, float3 start_local_position, float length);

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{

    float3 min_w = float3(-1.0, -1.0, -1.0);
    float3 max_w = float3(1.0, 1.0, 1.0);

    /*
    float3 eye_position = ps.view_position;
    float4 eye_ray = float4(normalize(input.position_world.xyz - eye_position), 0.0);
    float4 eye_ray_normalize_local = mul(mat.world_to_local, eye_ray);
    */

    float4 eye_local = mul(mat.world_to_local, float4(ps.view_position, 1.0));
    eye_local = eye_local / eye_local.w;
    float3 eye_ray = normalize(input.position_local.xyz - eye_local.xyz);

    float3 eye_ray_normalize_local_3 = 
    float3(
        (step(eye_ray.x, 0.0) - 0.5) * -2.0 * max(abs(eye_ray.x), 0.0000001),
        (step(eye_ray.y, 0.0) - 0.5) * -2.0 * max(abs(eye_ray.y), 0.0000001),
        (step(eye_ray.z, 0.0) - 0.5) * -2.0 * max(abs(eye_ray.z), 0.0000001)
);

    uint3 ray_step = step(eye_ray_normalize_local_3, float3(0.0, 0.0, 0.0));
    float3 target_w = ray_step * min_w + (1 - ray_step) * max_w;

    float3 len = abs((target_w - input.position_local.xyz) / eye_ray_normalize_local_3);

    float length_tem = min(
    len.x,
    min(len.y, len.z)
);

    float p_dist = linearize_z.Sample(ss, input.uv_screen).x - input.position_view.z;
    p_dist = min(p_dist, length_tem);

    output.color = float4(p_dist / sqrt(8.0), 0.0, 0.0, 1.0);







    //output.color = float4((-input.position_view.z + position_texture.Sample(ss, input.uv_screen).x) / 10.0, 0.0, 0.0, 1.0);
}