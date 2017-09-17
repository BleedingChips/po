#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11/shader/include/build_in_standard_input_type.hlsli"


float4 Texture2DSample(Texture2D Tex, SamplerState Sampler, float2 UV)
{
    return Tex.Sample(Sampler, UV);
}

Texture2D BaseShapeTexture : register(t0);
SamplerState BaseShapeTextureSamplerState : register(s0);

struct PropertyType
{
    float3 Scale;
    float3 Transform;
    float4 Factor;
};

cbuffer b0 : register(b0)
{
    PropertyType BaseShapeProperty;
}

Texture2D MoveMaskTexture : register(t1);
SamplerState MoveMaskTextureSamplerState : register(s1);

cbuffer b1 : register(b1)
{
    PropertyType MoveMaskProperty;
}

Texture2D MaskTexture : register(t2);
SamplerState MaskTextureSamplerState : register(s2);

cbuffer b2 : register(b2)
{
    PropertyType MaskProperty;
}

cbuffer b0 : register(b3)
{
    property_rendder_2d_for_3d pp;
}
cbuffer b1 : register(b4)
{
    property_local_transfer mat;
}
cbuffer b2 : register(b5)
{
    property_viewport_transfer ps;
}

Texture2D linearize_z : register(t3);
SamplerState ss : register(s3);

/*
    ray_start_point_local：体表面的像素的局部坐标在进行归一化到[0,1]之间后的坐标。
    ray_normalize_local：在世界坐标下的计算的射线向量，单位化后转换到局部坐标系。
    max_density：体素里1.0所表示的浓度值。
    worley_size：2d Worley Noise 所表示的3d 噪声的大小。
    target_depth：要计算最终的射线长度
    random_seed：产生Perlin Noise的随机数种子
*/

float4 implement(

// 基本形状属性
Texture2D BaseShape_T,
SamplerState BaseShape_TSampler,
float3 BaseShapeScale_F3,
float3 BaseShapeTransform_F3,
float4 BaseShapeValueFactor_F4,

// 移动的挖空属性
Texture2D MoveableMask_T,
SamplerState MoveableMask_TSampler,
float3 MoveableMaskSacle_F3,
float3 MoveableMaskTransform_F3,
float4 MoveableMaskValueFactor_F4,

// 静止的挖空属性
Texture2D Mask_T,
SamplerState Mask_TSampler,
float4 MaskValueFactor_F4,


float3 LocalPoint_F3,
float3 LocalRay_F3,
float3 MinWidth_F3,
float3 MaxWidth_F3,
float MaxDensity_F,
float ScreenDepth_F,
float Time_F,
float3 LocalLight_F3,
float3 Move_F3
);


void main(in standard_ps_input input, out standard_ps_output_transparent output)
{
    // 这个是不透明物体的深度，主要处理被不透明物体遮挡时候的问题
    float OpaqueDepth = 0.0;
    {
        //获取屏幕UV
        float2 screen_uv = get_uv_screen(input.uv_screen, input.position_sv);
        OpaqueDepth = linearize_z.Sample(ss, screen_uv).x;
    }

    // 计算像素点的深度与不透明物体的深度的最小值，计算射线的开始点
    float PixelMaxDepth = min(OpaqueDepth, input.position_view.z);

    // 计算视角射线向量
    float3 EyeRay = input.position_world.xyz - property_viewport_transfer_eye_world_position(ps);

    // 通过向量和深度比计算实际射线的实际开始点。
    float3 StartWorldPosition = EyeRay * (PixelMaxDepth / input.position_view.z) + property_viewport_transfer_eye_world_position(ps);

    // 单位化射线，反向，然后转换成局部坐标



    



    /*
    //世界坐标系下的射线向量，并单位化，升维
    float4 eye_ray_NOR = float4(normalize(input.position_world.xyz - property_viewport_transfer_eye_world_position(ps)), 0.0);

    //计算深度差
    float depth_length = max(linearize_z.Sample(ss, screen_uv).x - input.position_view.z, 0.0);

    //局部坐标系下的射线向量，降维，此时由于立方体本身的变换矩阵，所以不是单位化的
    eye_ray_NOR = mul(mat.world_to_local, eye_ray_NOR);
    */

    output.color = float4(1.0, 0.0, 0.0, 1.0);
    //float4(input.position_view.z / 10.0, 0.0, 0.0, 1.0);
    /*
    implement(
    ten,
    ss,
    input.position_local.xyz / input.position_local.w, 
    eye_ray_NOR.xyz, 
    pp.min_width, 
    pp.max_width, 
    pp.density, 
    depth_length, 
    ps.time * 0.0001, 
    float3(0.2, -1.0, 0.0), float3(1.0, 0.0, 0.0), 
    float3(50, 50, 50));
    */
}

/*
void ASMACRO COORDINATE_I3_TO_I3(out int3 out_I3, in int3 in_I3, in uint4 worley_U4)
{
    uint4 worley = worley_U4;
    int3 width_3 = int3(worley.x - 1, worley.y - 1, worley.z * worley.w - 1) * in_I3;
    out_I3 = width_3;
}

void ASMACRO COORDINATE_I3_TO_U2(out uint2 WorleyCoordinate_Out_U2, in int3 WorleyCoordinate_In_I3, in uint4 WorleySize_U4)
{
    int3 tem1 = WorleyCoordinate_In_I3;
    uint4 size = WorleySize_U4;
    uint3 noise_size = uint3(size.x, size.y, size.z * size.w) - 2;
    tem1 = abs(tem1);
    tem1 = tem1 % int3(noise_size * 2);
    uint3 tem2 = step(tem1, noise_size) * tem1 + step(noise_size, tem1) * (noise_size * 2.0 - tem1);
    WorleyCoordinate_Out_U2 = uint2((tem2.z % size.z) * size.x + tem2.x, (tem2.z / size.z) * size.y + tem2.y);
}

void ASMACRO LINEAR_SAMPLE(out float4 Sample_Result_F4, in float3 Coordinate_F3, in Texture2D Sample_Texture_T, in uint4 Worley_Size_U4)
{
    float3 Coord = Coordinate_F3;
    uint4 NoiseSize_INSIDE = Worley_Size_U4;
    Coord = Coord * float3(NoiseSize_INSIDE.x - 1, NoiseSize_INSIDE.y - 1, NoiseSize_INSIDE.z * NoiseSize_INSIDE.w - 1);
    int3 Coord_floor = floor(Coord);
    float3 Rate = Coord - Coord_floor;
    uint2 TextureCoord;
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor, NoiseSize_INSIDE);
    float4 tem = Sample_Texture_T[TextureCoord];
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(1, 0, 0), NoiseSize_INSIDE);
    float4 tem2 = Sample_Texture_T[TextureCoord];
    tem = lerp(tem, tem2, Rate.x);
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(0, 1, 0), NoiseSize_INSIDE);
    tem2 = Sample_Texture_T[TextureCoord];
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(1, 1, 0), NoiseSize_INSIDE);
    float4 tem3 = Sample_Texture_T[TextureCoord];
    tem2 = lerp(tem2, tem3, Rate.x);
    tem = lerp(tem, tem2, Rate.y);
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(0, 0, 1), NoiseSize_INSIDE);
    tem2 = Sample_Texture_T[TextureCoord];
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(1, 0, 1), NoiseSize_INSIDE);
    tem3 = Sample_Texture_T[TextureCoord];
    tem2 = lerp(tem2, tem3, Rate.x);
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(0, 1, 1), NoiseSize_INSIDE);
    tem3 = Sample_Texture_T[TextureCoord];
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(1, 1, 1), NoiseSize_INSIDE);
    float4 tem4 = Sample_Texture_T[TextureCoord];
    tem3 = lerp(tem3, tem4, Rate.x);
    tem2 = lerp(tem2, tem3, Rate.y);
    tem = lerp(tem, tem2, Rate.z);
    Sample_Result_F4 = tem;
}

void ASMACRO CALCULATE_NOISE(out float OutPut_F, in float3 Coordinate_F3, in Texture2D Sample_Texture_T, in uint4 Worley_Size_U4, in float3 Move_F3, in float Time_T, in float3 Scale_F3)
{
    float3 Scale = Scale_F3;
    float4 SampleValue;
    LINEAR_SAMPLE(SampleValue, Coordinate_F3 * Scale + Move_F3 * Time_T, Sample_Texture_T, Worley_Size_U4);
    OutPut_F = SampleValue.x * max(SampleValue.y, max(SampleValue.z, SampleValue.w));
}

void ASMACRO HG(out float Output_F, in float3 Light_F3, in float3 Ray_F3, in float G_F)
{
    float3 il = normalize(Light_F3);
    float3 rl = normalize(Ray_F3);
    float g = G_F;
    float g2 = g * g;
    Output_F = 0.5 * (1.0 - g2) / pow(1.0 + g2 - 2.0 * g * dot(il, rl), 1.5);
}
*/

/*
float4 Sample(Texture2D tex, SamplerState ss, float3 local)
{
    float4 size = float4(256, 256, 16, 16);
    float3 size_3 = float3(size.x, size.y, size.z * size.w);

    float2 final_uv = local 


    float3 tem = fmod(abs(local), 2.0);
    tem = min(tem, 2.0 - tem);
    
    float3 size_3 = float3(size.x, size.y, size.z * size.w);
    float3 tem2 = tem * size_3 - 2.0;
    float3 tem3 = float3(tem2.x, tem2.y, floor(tem2.z));
    float3 tem4 = float3(tem3.x, tem3.y, tem3.z + 1.0);
    float2 tem3_2 = tem3.x 
}
*/

float4 Sample_implement(Texture2D Tex, SamplerState SS, float3 SampleLocaltion, uint4 Block)
{
   
    float3 MirroLocation = abs(1.0 - fmod(abs(SampleLocaltion + 1.0), 2.0));

    float3 IntLoacation = MirroLocation * uint3(Block.x, Block.y, Block.z * Block.w);

    uint ZCount = Block.z * Block.w - 1;
    float ZChunk = MirroLocation.z * ZCount;
    uint ZChunkLast = floor(ZChunk);
    uint ZChunkMax = ZChunkLast + 1;
    float rate = ZChunk - ZChunkLast;
    ZChunkMax = step(ZChunkMax, ZCount) * (ZChunkMax);
    uint2 LastChunkXY = uint2(ZChunkLast % Block.z, ZChunkLast / float(Block.z));
    uint2 MaxChunkXY = uint2(ZChunkMax % Block.z, ZChunkMax / float(Block.z));
    float2 FinalLocationLast = float2(
    //(15 * MirroLocation.x + 1.0 + LastChunkXY.x) / float(Block.z),
    //(15 * MirroLocation.y + 1.0 + LastChunkXY.y) / float(Block.w)
    (MirroLocation.x * 0.98 + 0.01 + LastChunkXY.x) / float(Block.z),
    (MirroLocation.y * 0.98 + 0.01 + LastChunkXY.y) / float(Block.w)
    );
    float2 FinalLocationMax = float2(
    (MirroLocation.x * 0.98 + 0.01 + MaxChunkXY.x) / float(Block.z),
    (MirroLocation.y * 0.98 + 0.01 + MaxChunkXY.y) / float(Block.w)
    );

    float4 S1 = Texture2DSample(Tex, SS, FinalLocationLast);
    float4 S2 = Texture2DSample(Tex, SS, FinalLocationMax);

    return lerp(S1, S2, rate);

}







float4 implement(

// 基本形状属性
Texture2D BaseShape_T,
SamplerState BaseShape_TSampler,
float3 BaseShapeScale_F3,
float3 BaseShapeTransform_F3,
float4 BaseShapeValueFactor_F4,
uint4 BaseShapeSimulateSize_U4,

// 移动的挖空属性
Texture2D MoveableMask_T,
SamplerState MoveableMask_TSampler,
float3 MoveableMaskSacle_F3,
float3 MoveableMaskTransform_F3,
float4 MoveableMaskValueFactor_F4,
uint4 MoveableMaskSimulateSize_U4,

// 静止的挖空属性
Texture2D Mask_T,
SamplerState Mask_TSampler,
float4 MaskValueFactor_F4,
uint4 MaskSimulateSize_U4,

float3 LocalPoint_F3,
float3 LocalRay_F3,
float3 MaxWidthHeightDepthToSenter_F3,
float MaxDensity_F,
float ScreenDepth_F,
float Time_F,
float3 LocalLight_F3,
float3 LocalMove_F3
)
{

    //实际上也是长度在Local和在World下的比值
    float ray_local_length = length(LocalRay_F3);

    //计算射线穿过的距离，这里应该是World下的值（因为用的并不是单位化的Local Ray，而是变换过后的单位化World Ray，等于带上了缩放信息
    float ray_cross_length_world;
    {
        uint3 ray_step = step(float3(0.0, 0.0, 0.0), LocalRay_F3);
        float3 target_w = lerp(-MaxWidthHeightDepthToSenter_F3, MaxWidthHeightDepthToSenter_F3, ray_step);
        //防除零，给加个极小值
        float3 eye_ray_normalize_local_3 = ((step(LocalRay_F3, 0.0) - 0.5) * -2.0) * max(abs(LocalRay_F3), 0.0000000001);
        float3 len = abs((target_w - LocalPoint_F3) / LocalRay_F3);
        ray_cross_length_world = min(len.x, min(len.y, len.z));
    }

    //世界坐标系下的距离
    float target_depth = min(ray_cross_length_world, ScreenDepth_F);

    //局部坐标系下的距离
    float loacal_target_depth = target_depth * ray_local_length;

    //步进光线
    float3 ray = normalize(LocalRay_F3) * loacal_target_depth / 31.0;

    float ray_length = target_depth / 31.0;

    float Noise = 0.0;// = Sample_implement(Noise_T, Noise_TSampler, LocalPoint_F3 / 100.0 + float3(0.5, 0.5, 0.5), uint4(256, 256, 16, 16)).x;
    //CALCULATE_NOISE(Noise, LocalPoint_F3, Noise_T, NoiseSize, Move, Time_F, Scale_F3);
    //return float4(Noise, Noise, Noise,1.0);
    float last_density = Noise * MaxDensity_F;
    float decay = 0.0;
    float color = 0.0;

    float HG_V;
    //HG(HG_V, LocalLight_F, LocalRay_F3, 0.2);
    
    uint count = 0;
    for (count = 1; count < 31; ++count)
    {
        float3 poi = LocalPoint_F3 + ray * count;
        float3 cut_off = clamp(50 - abs(poi), 0.0, 10.0) / 10.0;
        float min_cut_off = min(cut_off.x, min(cut_off.y, cut_off.z));

        float4 BaseShapeValue = Sample_implement(BaseShape_T, BaseShape_TSampler, poi / 100.0 + float3(0.5, 0.5, 0.5), BaseShapeSimulateSize_U4);
        float3 normal_postion = (poi + MaxWidthHeightDepthToSenter_F3) / MaxWidthHeightDepthToSenter_F3;
        float3 shift_position = normal_postion + LocalMove_F3 * Time_F;

        float BaseShapeValueResult = 0.0;
        {
            float3 BaseShapePoistion = shift_position * BaseShapeScale_F3 + BaseShapeTransform_F3;
            float4 Result = Sample_implement(BaseShape_T, BaseShape_TSampler, BaseShapePoistion, BaseShapeSimulateSize_U4);
            BaseShapeValueResult = dot(Result, BaseShapeValueFactor_F4);
        }

        float MoveMaskValueResult = 0.0;
        {
            float3 BaseMoveMaskPoistion = shift_position * MoveableMaskSacle_F3 + MoveableMaskTransform_F3;
            float4 Result = Sample_implement(MoveableMask_T, MoveableMask_TSampler, BaseMoveMaskPoistion, MoveableMaskSimulateSize_U4);
            MoveMaskValueResult = dot(Result, MoveableMaskValueFactor_F4);
        }

        float MaskValueResult = 0.0;
        {
            float3 BaseMaskPostion = normal_postion;
            float4 Result = Sample_implement(Mask_T, Mask_TSampler, BaseMaskPostion, MaskSimulateSize_U4);
            MaskValueResult = dot(Result, MaskSimulateSize_U4);
        }




        float Noise = //Sample_implement(Noise_T, Noise_TSampler, poi / 50.0, uint4(256, 256, 16, 16)).x;
        //Sample_implement(Noise_T, Noise_TSampler, poi / 100.0 + float3(0.5, 0.5, 0.5) + float3(1.0, 1.0, 1.0) * Time_F, uint4(256, 256, 16, 16)).x * min_cut_off;
        //CALCULATE_NOISE(Noise, poi, Noise_T, NoiseSize, Move, Time_F, Scale_F3);

        clamp(BaseShapeValueResult - MoveMaskValueResult - MaskValueResult, 0.0, 1.0);



        float now_density = Noise * MaxDensity_F;
        float current_decay = (now_density + last_density) / 2.0 * ray_length;
        color = color + (1.0 - exp(-current_decay * 100.0)) * (exp(-decay)) * HG_V;
        decay = decay + current_decay;
        //decay = decay + now_density * ray_length;
        last_density = now_density;
    }

    float e_decay = exp(-decay);
    float e_decay2 = exp(-decay * 2.0);
    float inv_decay = 1.0 - e_decay;
    float inv_decay2 = (1.0 - e_decay2) * e_decay / inv_decay;
    float c = inv_decay2 * 2.0;
    return float4(1.0, 1.0, 1.0, inv_decay);
}







