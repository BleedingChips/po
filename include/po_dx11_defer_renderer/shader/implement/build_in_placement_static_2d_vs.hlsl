#include "../include/build_in_property_buffer_type.hlsli"
#include "../include/build_in_placement_ui_inout.hlsli"

cbuffer static_2d_input : register(b0)
{
    transfer_2d_static_buffer_t transfer;
};

cbuffer renderer3d : register(b1)
{
    renderer_3d_buffer_t renderer;
};


placemenet_ui_vs_output main(placement_ui_vs_input input)
{
    placemenet_ui_vs_output output;

    output.texcoord = input.tex;
    output.local_position = input.poi;
    output.out_poisition = float4(input.poi, 0.5, 1.0);
    
    float2 cal_point = (input.poi - transfer.scale_and_center.zw) * transfer.scale_and_center.xy + transfer.scale_and_center.zw;

    //float2 cal_point = (input.poi + transfer.shift - scale_center) * scale + scale_center;
    float sa = sin(transfer.roll_and_center.x);
    float ca = cos(transfer.roll_and_center.x);
    cal_point = cal_point - transfer.roll_and_center.yz;


    cal_point = float2(
        cal_point.x * ca - cal_point.y * sa,
        cal_point.x * sa + cal_point.y * ca
    );

    cal_point = cal_point + transfer.roll_and_center.yz;

    cal_point = cal_point + transfer.shift;

    if(transfer.adapt_screen)
        output.out_poisition = float4(cal_point * float2(1.0 / renderer.rate, 1.0), 0.5, 1.0);
    else
        output.out_poisition = float4(cal_point, 0.5, 1.0);
    return output;
}