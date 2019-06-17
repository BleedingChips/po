#include "../../compute/shader/volume_cloud_compute_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/noise.hlsli"
StructuredBuffer<float3> sb : register(t0);
cbuffer b0 : register(b0)
{
    property_custom_random_point_f3 f3;
}

cbuffer b1 : register(b1)
{
    float4 data;
}


void main(in standard_ps_input input, out standard_ps_output_transparent output)
{
    float3 Pos = float3(input.uv, data.x);
    float Depth = 0.0;
    for (uint count = 0; count < f3.count; ++count)
    {
        float3 dir = Pos - sb[count];
        float Distance = distance(Pos, sb[count]);
        float Poewer = 0.0009 / dot(dir, dir);
        Depth += Poewer;
    }
    float color = step(data.z, Depth);
    output.color = float4(color, color, color, 1.0);

}