#include "volume_cloud_material_property.hlsli"
#include "../../../../../include/po_dx11_defer_renderer/shader/include/build_in_property_type.hlsli"
#include "../../../../../include/po_dx11_defer_renderer/shader/include/build_in_standard_input_type.hlsli"

#define ASMACRO

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

/*
    ray_start_point_local：体表面的像素的局部坐标在进行归一化到[0,1]之间后的坐标。
    ray_normalize_local：在世界坐标下的计算的射线向量，单位化后转换到局部坐标系。
    max_density：体素里1.0所表示的浓度值。
    worley_size：2d Worley Noise 所表示的3d 噪声的大小。
    target_depth：要计算最终的射线长度
    random_seed：产生Perlin Noise的随机数种子
*/
float4 implement(
Texture2D Noise_T,
float3 LocalPoint_F3,
float3 LocalRay_F3,
float3 MinWidth_F3,
float3 MaxWidth_F3,
float MaxDensity_F,
float ScreenDepth_F,
float Time_F,
float3 LocalLight_F,
float3 Move_F3
);

void main(in standard_ps_input input, out standard_ps_output_transparent output)
{
    //立方体在局部坐标系下的长宽高最大最小值
    float3 min_w = float3(-1.0, -1.0, -1.0);
    float3 max_w = float3(1.0, 1.0, 1.0);

    float2 screen_uv = (input.uv_screen / input.position_sv.w + 1.0) / 2.0;

    //世界坐标系下的射线向量，并单位化，升维
    float4 eye_ray_NOR = float4(normalize(input.position_world.xyz - ps.view_position), 0.0);

    //计算深度差
    float depth_length = max(linearize_z.Sample(ss, screen_uv).x - input.position_view.z, 0.0);

    //局部坐标系下的射线向量，降维，此时由于立方体本身的变换矩阵，所以不是单位化的
    eye_ray_NOR = mul(mat.world_to_local, eye_ray_NOR);

    output.color = //float4(linearize_z.Sample(ss, screen_uv).x / 1000, 0.0, 0.0, 1.0);
    //float4(input.uv_screen, 0.0, 1.0);
    implement(
    ten,
    input.position_local.xyz / input.position_local.w, 
    eye_ray_NOR.xyz, 
    min_w, 
    max_w, 
    pp.density, 
    depth_length, 
    ps.time * 0.0001, 
    float3(0.2, -1.0, 0.0), float3(1.0, 0.0, 0.0));
}


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
    uint4 NoiseSize = Worley_Size_U4;
    Coord = Coord * float3(NoiseSize.x - 1, NoiseSize.y - 1, NoiseSize.z * NoiseSize.w - 1);
    int3 Coord_floor = floor(Coord);
    float3 Rate = Coord - Coord_floor;
    uint2 TextureCoord;
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor, NoiseSize);
    float4 tem = Sample_Texture_T[TextureCoord];
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(1, 0, 0), NoiseSize);
    float4 tem2 = Sample_Texture_T[TextureCoord];
    tem = lerp(tem, tem2, Rate.x);
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(0, 1, 0), NoiseSize);
    tem2 = Sample_Texture_T[TextureCoord];
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(1, 1, 0), NoiseSize);
    float4 tem3 = Sample_Texture_T[TextureCoord];
    tem2 = lerp(tem2, tem3, Rate.x);
    tem = lerp(tem, tem2, Rate.y);
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(0, 0, 1), NoiseSize);
    tem2 = Sample_Texture_T[TextureCoord];
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(1, 0, 1), NoiseSize);
    tem3 = Sample_Texture_T[TextureCoord];
    tem2 = lerp(tem2, tem3, Rate.x);
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(0, 1, 1), NoiseSize);
    tem3 = Sample_Texture_T[TextureCoord];
    COORDINATE_I3_TO_U2(TextureCoord, Coord_floor + int3(1, 1, 1), NoiseSize);
    float4 tem4 = Sample_Texture_T[TextureCoord];
    tem3 = lerp(tem3, tem4, Rate.x);
    tem2 = lerp(tem2, tem3, Rate.y);
    tem = lerp(tem, tem2, Rate.z);
    Sample_Result_F4 = tem;
}

void ASMACRO CALCULATE_NOISE(out float OutPut_F, in float3 Coordinate_F3, in Texture2D Sample_Texture_T, in uint4 Worley_Size_U4, in float3 Move_F3, in float Time_T)
{
    float4 SampleValue;
    LINEAR_SAMPLE(SampleValue, Coordinate_F3 / 2.0 + Move_F3 * Time_T, Sample_Texture_T, Worley_Size_U4);
    OutPut_F = SampleValue.x * (SampleValue.y / 2.0 + SampleValue.z / 4.0 + SampleValue.w / 4.0);
}

void ASMACRO HG(out float Output_F, in float3 Light_F3, in float3 Ray_F3, in float G_F)
{
    float3 il = normalize(Light_F3);
    float3 rl = normalize(Ray_F3);
    float g = G_F;
    float g2 = g * g;
    Output_F = 0.5 * (1.0 - g2) / pow(1.0 + g2 - 2.0 * g * dot(il, rl), 1.5);
}


float4 implement(
Texture2D Noise_T,
float3 LocalPoint_F3,
float3 LocalRay_F3,
float3 MinWidth_F3,
float3 MaxWidth_F3,
float MaxDensity_F,
float ScreenDepth_F,
float Time_F,
float3 LocalLight_F,
float3 Move_F3
)
{
    uint4 NoiseSize = uint4(256, 256, 16, 16);

    float3 Move = normalize(Move_F3);

    //实际上也是长度在Local和在World下的比值
    float ray_local_length = length(LocalRay_F3);

    //计算射线穿过的距离，这里应该是World下的值（因为用的并不是单位化的Local Ray，而是变换过后的单位化World Ray，等于带上了缩放信息
    float ray_cross_length_world;
    {
        uint3 ray_step = step(LocalRay_F3, float3(0.0, 0.0, 0.0));
        float3 target_w = ray_step * MinWidth_F3 + (1 - ray_step) * MaxWidth_F3;
        //防除零，给加个极小值
        float3 eye_ray_normalize_local_3 = ((step(LocalRay_F3, 0.0) - 0.5) * -2.0) * max(abs(LocalRay_F3), 0.0000000001);
        float3 len = abs((target_w - LocalPoint_F3) / LocalRay_F3);
        ray_cross_length_world = min(len.x, min(len.y, len.z));
    }

    //世界坐标系下的距离
    float target_depth = min(ray_cross_length_world, ScreenDepth_F);

    //return float4(target_depth / sqrt(12), 0.0, 0.0, 1.0);

    //局部坐标系下的距离
    float loacal_target_depth = target_depth * ray_local_length;

    //步进光线
    float3 ray = normalize(LocalRay_F3) * loacal_target_depth / 31.0;

    float ray_length = target_depth / 31.0;

    float Noise;
    CALCULATE_NOISE(Noise, LocalPoint_F3, Noise_T, NoiseSize, Move, Time_F);

    float last_density = Noise * MaxDensity_F;
    float decay = 0.0;
    float color = 0.0;

    float HG_V;
    HG(HG_V, LocalLight_F, LocalRay_F3, 0.2);
    
    uint count = 0;
    for (count = 1; count < 31; ++count)
    {
        float3 poi = LocalPoint_F3 + ray * count;
        float Noise;
        CALCULATE_NOISE(Noise, poi, Noise_T, NoiseSize, Move, Time_F);
        float now_density = Noise * MaxDensity_F;
        float current_decay = (now_density + last_density) / 2.0 * ray_length;
        color = color + (1.0 - exp(-current_decay * 100.0)) * (exp(-decay)) * HG_V;
        decay = decay + current_decay;
        //decay = decay + now_density * ray_length;
        last_density = now_density;
    }

    float e_decay = exp(-decay);
    float inv_decay = 1.0 - e_decay;
    float inv_decay2 = inv_decay;
    float c = clamp((1.0 + exp(-decay)) * HG_V, 0.0, 1.0);
    return float4(1.0, 1.0, 1.0, inv_decay);
}